extends Node3D

# DR. DRIVING 3D - endless highway, chase camera, menu, traffic and generated sounds.
# Godot 4.x. No external assets required.

const ROAD_WIDTH: float = 12.0
const ROAD_LOOP_LENGTH: float = 520.0
const LANE_X: Array[float] = [-4.0, 0.0, 4.0]
const MAX_SPEED: float = 42.0
const ACCELERATION: float = 18.0
const BRAKING: float = 28.0

var rng := RandomNumberGenerator.new()
var player_car: Node3D
var camera: Camera3D
var hud: Label
var menu_panel: PanelContainer
var game_over_panel: PanelContainer
var speed_label: Label
var coin_label: Label

var traffic: Array[Node3D] = []
var coins: Array[Node3D] = []
var road_dashes: Array[Node3D] = []
var scenery_recycle_nodes: Array[Node3D] = []
var bird_nodes: Array[Node3D] = []

var engine_player: AudioStreamPlayer
var coin_player: AudioStreamPlayer
var crash_player: AudioStreamPlayer

var player_x: float = 0.0
var speed: float = 0.0
var score: int = 0
var coins_collected: int = 0
var crashed: bool = false
var gameplay_enabled: bool = false
var engine_base_pitch: float = 0.7
var f11_was_down: bool = false


func _ready() -> void:
	rng.randomize()
	# Apply fullscreen after the game window has initialized.
	call_deferred("_apply_fullscreen")
	create_world()
	create_road()
	create_scenery()
	create_player()
	create_traffic()
	create_coins()
	create_audio()
	create_ui()
	show_menu()



func _apply_fullscreen() -> void:
	# Use the laptop's active display resolution, without hard-coding 1080p.
	var window := get_window()
	if window:
		window.mode = Window.MODE_FULLSCREEN
		window.borderless = true
	DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN)


# ------------------------------------------------------------
# Materials and procedural primitives
# ------------------------------------------------------------

func make_material(color: Color, roughness: float = 0.8, metallic: float = 0.0) -> StandardMaterial3D:
	var m := StandardMaterial3D.new()
	m.albedo_color = color
	m.roughness = roughness
	m.metallic = metallic
	return m


func make_emissive(color: Color) -> StandardMaterial3D:
	var m := make_material(color, 0.3)
	m.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	m.emission_enabled = true
	m.emission = color
	m.emission_energy_multiplier = 1.3
	return m


func add_box(parent: Node3D, size: Vector3, pos: Vector3, mat: Material) -> MeshInstance3D:
	var instance := MeshInstance3D.new()
	var mesh := BoxMesh.new()
	mesh.size = size
	instance.mesh = mesh
	instance.position = pos
	instance.material_override = mat
	parent.add_child(instance)
	return instance


func add_sphere(parent: Node3D, radius: float, pos: Vector3, mat: Material) -> MeshInstance3D:
	var instance := MeshInstance3D.new()
	var mesh := SphereMesh.new()
	mesh.radius = radius
	mesh.height = radius * 2.0
	mesh.radial_segments = 12
	mesh.rings = 8
	instance.mesh = mesh
	instance.position = pos
	instance.material_override = mat
	parent.add_child(instance)
	return instance


func add_cylinder(parent: Node3D, top_radius: float, bottom_radius: float, height: float, pos: Vector3, mat: Material) -> MeshInstance3D:
	var instance := MeshInstance3D.new()
	var mesh := CylinderMesh.new()
	mesh.top_radius = top_radius
	mesh.bottom_radius = bottom_radius
	mesh.height = height
	mesh.radial_segments = 12
	instance.mesh = mesh
	instance.position = pos
	instance.material_override = mat
	parent.add_child(instance)
	return instance


func register_recycling_group(group: Node3D) -> void:
	# Move the group's children into local coordinates so the group's Z
	# position represents the actual scenery segment for endless recycling.
	if group.get_child_count() > 0:
		var origin_z: float = group.get_child(0).position.z
		for child in group.get_children():
			if child is Node3D:
				child.position.z -= origin_z
		group.position.z = origin_z
	scenery_recycle_nodes.append(group)


# ------------------------------------------------------------
# World, sky, lighting and ground
# ------------------------------------------------------------

func create_world() -> void:
	var world := WorldEnvironment.new()
	var env := Environment.new()
	env.background_mode = Environment.BG_SKY

	var sky := Sky.new()
	var sky_mat := ProceduralSkyMaterial.new()
	sky_mat.sky_top_color = Color(0.08, 0.22, 0.48)
	sky_mat.sky_horizon_color = Color(0.63, 0.79, 0.92)
	sky_mat.ground_bottom_color = Color(0.16, 0.23, 0.15)
	sky_mat.ground_horizon_color = Color(0.56, 0.66, 0.64)
	sky_mat.sun_angle_max = 25.0
	sky.sky_material = sky_mat
	env.sky = sky
	env.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	env.ambient_light_color = Color(0.72, 0.78, 0.88)
	env.ambient_light_energy = 0.75
	env.tonemap_mode = Environment.TONE_MAPPER_FILMIC
	env.tonemap_exposure = 1.05
	env.fog_enabled = true
	env.fog_light_color = Color(0.65, 0.77, 0.88)
	env.fog_density = 0.0011
	world.environment = env
	add_child(world)

	var sun := DirectionalLight3D.new()
	sun.rotation_degrees = Vector3(-48.0, -28.0, 0.0)
	sun.light_energy = 1.35
	sun.shadow_enabled = true
	add_child(sun)

	add_box(
		self, Vector3(260.0, 0.5, 25000.0),
		Vector3(0.0, -0.42, -12400.0),
		make_material(Color(0.20, 0.39, 0.15))
	)

	var sun_disc := add_sphere(
		self, 4.0, Vector3(-75.0, 68.0, -220.0),
		make_emissive(Color(1.0, 0.78, 0.38))
	)
	sun_disc.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF


# ------------------------------------------------------------
# Road and endlessly recycled lane markings
# ------------------------------------------------------------

func create_road() -> void:
	add_box(
		self, Vector3(ROAD_WIDTH, 0.18, 25000.0),
		Vector3(0.0, -0.08, -12400.0),
		make_material(Color(0.105, 0.115, 0.13), 0.96)
	)

	var edge_mat := make_material(Color(0.92, 0.91, 0.83))
	var yellow_mat := make_material(Color(1.0, 0.72, 0.08))
	var white_mat := make_material(Color(0.92, 0.93, 0.91))

	for side in [-1.0, 1.0]:
		add_box(
			self, Vector3(0.16, 0.035, 25000.0),
			Vector3(float(side) * 5.58, 0.025, -12400.0), edge_mat
		)

	for i in range(44):
		var z := -float(i) * 12.0 - 4.0
		var dash := add_box(
			self, Vector3(0.14, 0.035, 5.0),
			Vector3(0.0, 0.035, z), yellow_mat
		)
		road_dashes.append(dash)

		for lane_x in [-2.0, 2.0]:
			add_box(
				self, Vector3(0.07, 0.03, 5.0),
				Vector3(float(lane_x), 0.03, z), white_mat
			)


# ------------------------------------------------------------
# Scenic environment: mountains, river, ponds, forest, grass,
# lamps, signs and birds. Groups recycle as the car drives.
# ------------------------------------------------------------

func create_scenery() -> void:
	var mountain_colors: Array[Color] = [
		Color(0.28, 0.36, 0.43),
		Color(0.35, 0.45, 0.48),
		Color(0.42, 0.50, 0.49),
		Color(0.30, 0.40, 0.39)
	]

	for i in range(24):
		var group := Node3D.new()
		add_child(group)
		var mountain := MeshInstance3D.new()
		var mesh := CylinderMesh.new()
		mesh.top_radius = 0.0
		mesh.bottom_radius = rng.randf_range(16.0, 29.0)
		mesh.height = rng.randf_range(28.0, 52.0)
		mesh.radial_segments = 7
		mountain.mesh = mesh
		var side := -1.0 if i % 2 == 0 else 1.0
		mountain.position = Vector3(
			side * rng.randf_range(45.0, 100.0),
			mesh.height * 0.35,
			-float(i) * 24.0 - 70.0
		)
		mountain.rotation.y = rng.randf_range(0.0, 3.0)
		mountain.material_override = make_material(mountain_colors[i % mountain_colors.size()])
		group.add_child(mountain)
		register_recycling_group(group)

	# River segments winding through the valley.
	var river_mat := make_material(Color(0.08, 0.36, 0.57), 0.22, 0.1)
	var bank_mat := make_material(Color(0.52, 0.46, 0.30))
	for i in range(24):
		var group := Node3D.new()
		var x := 32.0 + sin(float(i) * 0.5) * 7.0
		var z := -float(i) * 22.0 - 10.0
		add_box(group, Vector3(14.0, 0.10, 24.0), Vector3(x, -0.16, z), bank_mat)
		add_box(group, Vector3(10.5, 0.07, 22.0), Vector3(x, -0.08, z), river_mat)
		add_child(group)
		register_recycling_group(group)

	# Ponds on both sides.
	var pond_mat := make_material(Color(0.08, 0.39, 0.60), 0.2, 0.05)
	for i in range(8):
		var group := Node3D.new()
		var pond := MeshInstance3D.new()
		var pond_mesh := CylinderMesh.new()
		pond_mesh.top_radius = rng.randf_range(3.0, 5.0)
		pond_mesh.bottom_radius = pond_mesh.top_radius
		pond_mesh.height = 0.10
		pond_mesh.radial_segments = 24
		pond.mesh = pond_mesh
		pond.position = Vector3(-23.0 if i % 2 == 0 else 24.0, -0.12, -float(i) * 58.0 - 30.0)
		pond.material_override = pond_mat
		group.add_child(pond)
		add_child(group)
		register_recycling_group(group)

	var trunk_mat := make_material(Color(0.25, 0.13, 0.06))
	var leaf_mats: Array[Material] = [
		make_material(Color(0.05, 0.22, 0.07)),
		make_material(Color(0.08, 0.32, 0.10)),
		make_material(Color(0.15, 0.40, 0.12))
	]

	# Trees: each tree is a group so it can be recycled together.
	for i in range(120):
		var group := Node3D.new()
		var side := -1.0 if i % 2 == 0 else 1.0
		var x := side * rng.randf_range(8.0, 23.0)
		var z := -rng.randf_range(0.0, 470.0)
		var height := rng.randf_range(1.8, 3.6)
		add_cylinder(group, 0.14, 0.25, height, Vector3(x, height * 0.5, z), trunk_mat)
		add_sphere(group, rng.randf_range(0.9, 1.55), Vector3(x, height + 0.45, z), leaf_mats[i % leaf_mats.size()])
		add_child(group)
		register_recycling_group(group)

	# Grass and small plants.
	var grass_mats: Array[Material] = [
		make_material(Color(0.16, 0.38, 0.10)),
		make_material(Color(0.27, 0.49, 0.13)),
		make_material(Color(0.12, 0.31, 0.08))
	]
	for i in range(260):
		var group := Node3D.new()
		var side := -1.0 if i % 2 == 0 else 1.0
		var x := side * rng.randf_range(6.4, 18.0)
		var z := -rng.randf_range(0.0, 480.0)
		var blade := add_box(
			group, Vector3(0.12, rng.randf_range(0.12, 0.45), 0.12),
			Vector3(x, 0.02, z), grass_mats[i % grass_mats.size()]
		)
		blade.rotation.z = rng.randf_range(-0.35, 0.35)
		add_child(group)
		register_recycling_group(group)

	# Lamp posts and lamps.
	var pole_mat := make_material(Color(0.20, 0.22, 0.24), 0.4, 0.35)
	var lamp_mat := make_emissive(Color(1.0, 0.86, 0.58))
	for i in range(22):
		var group := Node3D.new()
		var z := -float(i) * 22.0 - 10.0
		for side in [-1.0, 1.0]:
			var x := float(side) * 7.0
			add_box(group, Vector3(0.12, 4.0, 0.12), Vector3(x, 2.0, z), pole_mat)
			add_box(group, Vector3(0.65, 0.15, 0.28), Vector3(x, 4.0, z), lamp_mat)
		add_child(group)
		register_recycling_group(group)

	# Roadside signs.
	var sign_green := make_material(Color(0.025, 0.27, 0.10))
	for i in range(10):
		var group := Node3D.new()
		var side := -1.0 if i % 2 == 0 else 1.0
		var x := side * 7.0
		var z := -float(i) * 48.0 - 25.0
		add_box(group, Vector3(0.12, 2.0, 0.12), Vector3(x, 1.0, z), pole_mat)
		add_box(group, Vector3(1.7, 0.9, 0.14), Vector3(x, 2.0, z), sign_green)
		add_child(group)
		register_recycling_group(group)

	# Small birds in the sky.
	var bird_mat := make_material(Color(0.08, 0.10, 0.13))
	for i in range(10):
		var bird := Node3D.new()
		bird.position = Vector3(rng.randf_range(-24.0, 24.0), rng.randf_range(13.0, 22.0), -rng.randf_range(25.0, 330.0))
		add_child(bird)
		bird_nodes.append(bird)
		var left := add_box(bird, Vector3(0.8, 0.05, 0.16), Vector3(-0.32, 0.0, 0.0), bird_mat)
		var right := add_box(bird, Vector3(0.8, 0.05, 0.16), Vector3(0.32, 0.0, 0.0), bird_mat)
		left.rotation.z = -0.35
		right.rotation.z = 0.35


# ------------------------------------------------------------
# Player car, chase camera and traffic
# ------------------------------------------------------------

func build_car(parent: Node3D, color: Color) -> Node3D:
	var car := Node3D.new()
	parent.add_child(car)

	var body := make_material(color, 0.28, 0.25)
	var glass := make_material(Color(0.055, 0.12, 0.17), 0.18, 0.1)
	var tire := make_material(Color(0.025, 0.025, 0.028))
	var rim := make_material(Color(0.62, 0.65, 0.68), 0.3, 0.6)
	var headlight := make_emissive(Color(1.0, 0.91, 0.65))
	var taillight := make_emissive(Color(0.95, 0.025, 0.02))

	add_box(car, Vector3(1.72, 0.54, 3.2), Vector3(0.0, 0.57, 0.0), body)
	add_box(car, Vector3(1.22, 0.62, 1.48), Vector3(0.0, 1.08, -0.22), glass)
	add_box(car, Vector3(1.24, 0.08, 1.48), Vector3(0.0, 1.39, -0.22), body)
	add_box(car, Vector3(1.35, 0.08, 0.35), Vector3(0.0, 0.88, -1.12), body)

	for side in [-1.0, 1.0]:
		var sx := float(side) * 0.91
		for wz in [-1.0, 1.0]:
			var wheel := add_cylinder(car, 0.33, 0.33, 0.20, Vector3(sx, 0.34, float(wz)), tire)
			wheel.rotation.z = PI / 2.0
			var wheel_rim := add_cylinder(car, 0.17, 0.17, 0.215, Vector3(sx * 1.02, 0.34, float(wz)), rim)
			wheel_rim.rotation.z = PI / 2.0

		add_box(car, Vector3(0.31, 0.13, 0.06), Vector3(float(side) * 0.51, 0.66, 1.62), headlight)
		add_box(car, Vector3(0.28, 0.13, 0.06), Vector3(float(side) * 0.51, 0.66, -1.62), taillight)

	# Side mirrors
	for side in [-1.0, 1.0]:
		add_box(car, Vector3(0.25, 0.12, 0.18), Vector3(float(side) * 0.9, 1.0, 0.48), body)

	return car


func create_player() -> void:
	player_car = build_car(self, Color(0.025, 0.22, 0.92))
	player_car.position = Vector3(0.0, 0.0, 0.0)
	# The car model's front is +Z; rotate it so its front faces the road ahead (-Z).
	player_car.rotation.y = PI

	camera = Camera3D.new()
	camera.position = Vector3(0.0, 4.4, 9.4)
	camera.fov = 72.0
	camera.near = 0.1
	camera.far = 800.0
	add_child(camera)
	camera.global_position = player_car.global_position + Vector3(0.0, 4.4, 9.4)
	camera.look_at(player_car.global_position + Vector3(0.0, 0.9, -9.0), Vector3.UP)
	camera.current = true


func create_traffic() -> void:
	var colors: Array[Color] = [
		Color(0.85, 0.07, 0.04),
		Color(0.95, 0.42, 0.03),
		Color(0.92, 0.76, 0.06),
		Color(0.40, 0.12, 0.70),
		Color(0.03, 0.58, 0.47),
		Color(0.82, 0.84, 0.86),
		Color(0.10, 0.11, 0.13)
	]

	for i in range(7):
		var car := build_car(self, colors[i])
		car.position = Vector3(LANE_X[i % LANE_X.size()], 0.0, -45.0 - float(i) * 48.0)
		traffic.append(car)


# ------------------------------------------------------------
# Coins
# ------------------------------------------------------------

func create_coins() -> void:
	var gold := make_material(Color(1.0, 0.66, 0.035), 0.22, 0.35)
	for i in range(16):
		var coin := MeshInstance3D.new()
		var mesh := CylinderMesh.new()
		mesh.top_radius = 0.48
		mesh.bottom_radius = 0.48
		mesh.height = 0.13
		mesh.radial_segments = 24
		coin.mesh = mesh
		coin.material_override = gold
		coin.position = Vector3(LANE_X[i % LANE_X.size()], 1.15, -25.0 - float(i) * 27.0)
		add_child(coin)
		coins.append(coin)


# ------------------------------------------------------------
# Procedurally generated audio: engine loop, coin chime, crash
# ------------------------------------------------------------

func make_tone_wav(duration: float, start_freq: float, end_freq: float, volume: float, looped: bool) -> AudioStreamWAV:
	var sample_rate := 22050
	var sample_count := int(duration * float(sample_rate))
	var bytes := PackedByteArray()
	bytes.resize(sample_count * 2)

	for i in range(sample_count):
		var t := float(i) / float(sample_rate)
		var progress := float(i) / float(maxi(1, sample_count - 1))
		var freq := lerpf(start_freq, end_freq, progress)
		var envelope := 1.0
		if not looped:
			envelope = minf(1.0, t * 35.0) * pow(maxf(0.0, 1.0 - progress), 0.7)

		var sample_value := sin(TAU * freq * t) * 0.65
		sample_value += sin(TAU * freq * 2.02 * t) * 0.20
		sample_value += sin(TAU * freq * 0.5 * t) * 0.15
		var sample_int := int(clampf(sample_value * volume * envelope, -1.0, 1.0) * 32767.0)
		if sample_int < 0:
			sample_int += 65536
		bytes[i * 2] = sample_int & 255
		bytes[i * 2 + 1] = (sample_int >> 8) & 255

	var wav := AudioStreamWAV.new()
	wav.format = AudioStreamWAV.FORMAT_16_BITS
	wav.mix_rate = sample_rate
	wav.stereo = false
	wav.data = bytes
	if looped:
		wav.loop_mode = AudioStreamWAV.LOOP_FORWARD
		wav.loop_begin = 0
		wav.loop_end = sample_count
	return wav


func create_audio() -> void:
	engine_player = AudioStreamPlayer.new()
	engine_player.stream = make_tone_wav(1.0, 48.0, 64.0, 0.13, true)
	engine_player.volume_db = -12.0
	add_child(engine_player)
	engine_player.play()

	coin_player = AudioStreamPlayer.new()
	coin_player.stream = make_tone_wav(0.24, 880.0, 1450.0, 0.55, false)
	coin_player.volume_db = -3.0
	add_child(coin_player)

	crash_player = AudioStreamPlayer.new()
	crash_player.stream = make_tone_wav(0.45, 150.0, 45.0, 0.65, false)
	crash_player.volume_db = -1.0
	add_child(crash_player)


# ------------------------------------------------------------
# Menu, HUD and game-over screen
# ------------------------------------------------------------

func create_ui() -> void:
	var canvas := CanvasLayer.new()
	canvas.layer = 10
	add_child(canvas)

	hud = Label.new()
	hud.position = Vector2(22.0, 18.0)
	hud.add_theme_font_size_override("font_size", 22)
	hud.add_theme_color_override("font_color", Color.WHITE)
	hud.add_theme_color_override("font_shadow_color", Color.BLACK)
	hud.add_theme_constant_override("shadow_offset_x", 2)
	hud.add_theme_constant_override("shadow_offset_y", 2)
	canvas.add_child(hud)

	# Menu backdrop
	var backdrop := ColorRect.new()
	backdrop.color = Color(0.015, 0.035, 0.06, 0.72)
	backdrop.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	canvas.add_child(backdrop)

	menu_panel = PanelContainer.new()
	menu_panel.anchor_left = 0.5
	menu_panel.anchor_top = 0.5
	menu_panel.anchor_right = 0.5
	menu_panel.anchor_bottom = 0.5
	menu_panel.offset_left = -210.0
	menu_panel.offset_top = -190.0
	menu_panel.offset_right = 210.0
	menu_panel.offset_bottom = 190.0
	canvas.add_child(menu_panel)

	var menu_box := VBoxContainer.new()
	menu_box.add_theme_constant_override("separation", 14)
	menu_panel.add_child(menu_box)

	var title := Label.new()
	title.text = "DR. DRIVING 3D"
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	title.add_theme_font_size_override("font_size", 30)
	menu_box.add_child(title)

	var subtitle := Label.new()
	subtitle.text = "SCENIC HIGHWAY DRIVE"
	subtitle.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	subtitle.add_theme_font_size_override("font_size", 15)
	menu_box.add_child(subtitle)

	var start_button := Button.new()
	start_button.text = "START DRIVING"
	start_button.custom_minimum_size = Vector2(0.0, 48.0)
	start_button.pressed.connect(start_game)
	menu_box.add_child(start_button)

	var controls := Label.new()
	controls.text = "W / Up: Accelerate\nS / Down: Brake\nA / Left and D / Right: Steer\nCollect gold coins. Avoid traffic."
	controls.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	controls.add_theme_font_size_override("font_size", 16)
	menu_box.add_child(controls)

	var quit_button := Button.new()
	quit_button.text = "QUIT"
	quit_button.custom_minimum_size = Vector2(0.0, 40.0)
	quit_button.pressed.connect(func(): get_tree().quit())
	menu_box.add_child(quit_button)

	backdrop.visible = true
	menu_panel.visible = true
	menu_panel.set_meta("backdrop", backdrop)

	game_over_panel = PanelContainer.new()
	game_over_panel.anchor_left = 0.5
	game_over_panel.anchor_top = 0.5
	game_over_panel.anchor_right = 0.5
	game_over_panel.anchor_bottom = 0.5
	game_over_panel.offset_left = -190.0
	game_over_panel.offset_top = -120.0
	game_over_panel.offset_right = 190.0
	game_over_panel.offset_bottom = 120.0
	canvas.add_child(game_over_panel)

	var over_box := VBoxContainer.new()
	over_box.add_theme_constant_override("separation", 14)
	game_over_panel.add_child(over_box)

	var over_title := Label.new()
	over_title.text = "CRASH!"
	over_title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	over_title.add_theme_font_size_override("font_size", 30)
	over_box.add_child(over_title)

	var retry_button := Button.new()
	retry_button.text = "PLAY AGAIN"
	retry_button.custom_minimum_size = Vector2(0.0, 48.0)
	retry_button.pressed.connect(restart_game)
	over_box.add_child(retry_button)

	var menu_button := Button.new()
	menu_button.text = "MAIN MENU"
	menu_button.custom_minimum_size = Vector2(0.0, 40.0)
	menu_button.pressed.connect(show_menu)
	over_box.add_child(menu_button)

	game_over_panel.visible = false
	update_hud()


func show_menu() -> void:
	gameplay_enabled = false
	crashed = false
	if menu_panel:
		menu_panel.visible = true
		var backdrop = menu_panel.get_meta("backdrop", null)
		if backdrop is Control:
			backdrop.visible = true
	if game_over_panel:
		game_over_panel.visible = false
	if engine_player:
		engine_player.volume_db = -24.0
	Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
	update_hud()


func start_game() -> void:
	gameplay_enabled = true
	crashed = false
	if menu_panel:
		menu_panel.visible = false
		var backdrop = menu_panel.get_meta("backdrop", null)
		if backdrop is Control:
			backdrop.visible = false
	if game_over_panel:
		game_over_panel.visible = false
	if engine_player:
		engine_player.volume_db = -12.0
	Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
	get_viewport().gui_release_focus()


func show_game_over() -> void:
	crashed = true
	gameplay_enabled = false
	if game_over_panel:
		game_over_panel.visible = true
	if engine_player:
		engine_player.volume_db = -25.0
	if crash_player:
		crash_player.play()
	update_hud()


func update_hud() -> void:
	if not hud:
		return
	hud.text = "SCORE  %d      COINS  %d\nSPEED  %d km/h" % [
		score, coins_collected, roundi(speed * 5.0)
	]
	if crashed:
		hud.text += "\nCRASHED"


# ------------------------------------------------------------
# Main driving update: player actually travels down the road.
# Camera follows the player with smooth movement.
# ------------------------------------------------------------

func _process(delta: float) -> void:
	# F11 toggles between full-screen and windowed mode.
	var f11_down := Input.is_physical_key_pressed(KEY_F11)
	if f11_down and not f11_was_down:
		var window := get_window()
		if DisplayServer.window_get_mode() == DisplayServer.WINDOW_MODE_FULLSCREEN:
			DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_WINDOWED)
			DisplayServer.window_set_size(Vector2i(1280, 720))
			if window:
				window.borderless = false
		else:
			if window:
				window.borderless = true
				window.mode = Window.MODE_FULLSCREEN
			DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN)
	f11_was_down = f11_down

	if not is_instance_valid(player_car):
		return

	# Smooth chase camera follows the car through world space.
	var desired_camera_pos := player_car.global_position + Vector3(0.0, 4.4, 9.4)
	camera.global_position = camera.global_position.lerp(
		desired_camera_pos, 1.0 - exp(-5.0 * delta)
	)
	camera.look_at(
		player_car.global_position + Vector3(0.0, 0.9, -10.0),
		Vector3.UP
	)

	if not gameplay_enabled:
		# Gentle idle camera movement while menu is open.
		return

	if Input.is_physical_key_pressed(KEY_ESCAPE):
		show_menu()
		return

	if crashed:
		return

	var steering := 0.0
	if Input.is_physical_key_pressed(KEY_A) or Input.is_physical_key_pressed(KEY_LEFT):
		steering -= 1.0
	if Input.is_physical_key_pressed(KEY_D) or Input.is_physical_key_pressed(KEY_RIGHT):
		steering += 1.0

	if Input.is_physical_key_pressed(KEY_W) or Input.is_physical_key_pressed(KEY_UP):
		speed = minf(speed + ACCELERATION * delta, MAX_SPEED)
	elif Input.is_physical_key_pressed(KEY_S) or Input.is_physical_key_pressed(KEY_DOWN):
		speed = maxf(speed - BRAKING * delta, 0.0)
	else:
		speed = move_toward(speed, 15.0, 2.5 * delta)

	# Actual forward movement: the car travels along negative Z.
	player_x += steering * (4.0 + speed * 0.10) * delta
	player_x = clampf(player_x, -4.35, 4.35)
	player_car.position.x = player_x
	player_car.position.z -= speed * delta

	# Small body roll gives steering a sense of weight.
	player_car.rotation.z = lerpf(
		player_car.rotation.z, -steering * 0.055, 1.0 - exp(-7.0 * delta)
	)

	score += int(speed * delta)

	# Engine pitch follows speed.
	if engine_player:
		engine_player.pitch_scale = 0.75 + speed / MAX_SPEED * 0.65

	# Recycle lane markings as the car passes them.
	for dash in road_dashes:
		if dash.position.z > player_car.position.z + 8.0:
			dash.position.z -= ROAD_LOOP_LENGTH

	# Recycle roadside scenery behind the player.
	for group in scenery_recycle_nodes:
		if is_instance_valid(group) and group.global_position.z > player_car.position.z + 25.0:
			group.position.z -= ROAD_LOOP_LENGTH

	# ONCOMING TRAFFIC:
	# The player drives toward -Z; other vehicles drive toward +Z.
	# Their fronts point +Z, so they face the player head-on.
	for car in traffic:
		car.position.z += maxf(speed, 10.0) * delta * 0.85

		# After passing the player, respawn farther up the road.
		if car.position.z > player_car.position.z + 12.0:
			car.position.z = player_car.position.z - rng.randf_range(100.0, 240.0)
			car.position.x = LANE_X[rng.randi_range(0, LANE_X.size() - 1)]

		if (
			absf(car.position.x - player_x) < 1.55
			and absf(car.position.z - player_car.position.z) < 3.0
		):
			show_game_over()
			break

	if crashed:
		return

	# Coins move with the world perspective and trigger a chime.
	for coin in coins:
		coin.rotate_y(3.0 * delta)
		if coin.position.z > player_car.position.z + 12.0:
			coin.position.z = player_car.position.z - rng.randf_range(80.0, 180.0)
			coin.position.x = LANE_X[rng.randi_range(0, LANE_X.size() - 1)]

		if (
			absf(coin.position.x - player_x) < 1.35
			and absf(coin.position.z - player_car.position.z) < 2.0
		):
			coins_collected += 1
			score += 100
			coin.position.z = player_car.position.z - rng.randf_range(100.0, 190.0)
			coin.position.x = LANE_X[rng.randi_range(0, LANE_X.size() - 1)]
			if coin_player:
				coin_player.stop()
				coin_player.play()

	# Birds glide and recycle above the valley.
	for bird in bird_nodes:
		bird.position.x += sin(Time.get_ticks_msec() * 0.001 + bird.position.z) * delta * 0.7
		bird.position.z += speed * delta * 0.18
		if bird.position.z > player_car.position.z + 30.0:
			bird.position.z = player_car.position.z - rng.randf_range(180.0, 380.0)
			bird.position.x = rng.randf_range(-24.0, 24.0)

	update_hud()


func restart_game() -> void:
	get_tree().reload_current_scene()
