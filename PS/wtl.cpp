#include<iostream>
#include<cstdlib>
#include<ctime>

using namespace std;

void WaterTankLeakage(int Vi,int VM, int Vm, int Ri, int Ro, int t)
{
	bool alertTriggered = false;
	
	for(int i = 1; i <= t; i++) // t is in seconds for faster computation
	{
		Vi += (Ri - Ro);
		
		cout << "T=" << i << "s | Vi=" << Vi << endl;
		
		if(Vi <= Vm) 
		{
			cout << "At T=" << i << ", ";
			cout << "Trigger an Alarm and turn off the main inlet valve!\n";
			alertTriggered = true;
			break;
		}
		
		if(Vi >= VM)
		{
			cout << "At T=" << i << ", ";
			cout << "Shutdown the pump to avoid overflow!\n";
			alertTriggered = true;
			break;
		}
		
		if(!alertTriggered)
		{
			cout << "Water will remain in the tank at Vi=" << Vi << "litres." << endl;
		}
		cout << endl;
	}
	return;
}

int main()
{
	int Vi, VM, Vm, Ri, Ro, t;
	cout << "\n--------------- Water Tank Leakage ---------------\n";
	cout << "Enter time 't' in seconds: ";
	if(!(cin >> t) || t <= 0)
	{
		cout << "Invalid Time Input\n";
		return 1;
	}
	
	srand(time(nullptr));
	
	Vm = (rand()%20 + 5)*1000;
	VM = (rand()%50 + 20)*1000;
	Vi = (rand()%(VM-Vm));
	Ri = (rand()%10 + 1)*1000;
	Ro = (rand()%10 + 1)*1000;
	
	cout << "\nHere,\n"; 
	cout << "time=" << t << " seconds" <<endl;
	cout << "Maximum Volume="<< VM << " litres" <<endl;
	cout << "Minimum Volume="<< Vm << " litres" <<endl;
	cout << "Initial Volume="<< Vi << " litres" <<endl;
	cout << "inflow rate="<< Ri << " litre/second" <<endl;
	cout << "outflow rate="<< Ro << " litre/second" <<endl << endl;
	
	WaterTankLeakage(Vi, VM, Vm, Ri, Ro, t);
	return 0;
}
