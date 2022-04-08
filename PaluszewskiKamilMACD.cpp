// KAMIL PALUSZEWSKI 180194
#define  _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

#define ZAKRES 1000

double AVGkroczaca(int okres, int nr_notowania, double* notowania)
{
	double gora = 0.0; 
	double dol = 0.0;
	double a = 2 / (okres + 1);
	double wspolczynnik = 1 - a;

	for (int i = 0; i <= okres; i++)
	{
		int rozpatrywane = nr_notowania - i;
		if (rozpatrywane >= 0)
		{
			double tmp = pow(wspolczynnik, i);
			dol += 1.0 * tmp;
			tmp *= notowania[rozpatrywane];
			gora += tmp;	
		}
	}
	return gora / dol;
}

int Wczytaj(const char* pobrane, double* notowania)
{
	ifstream inFile;
	char nazwa[512] = "notowania/";
	strcat(nazwa, pobrane);
	strcat(nazwa, ".csv");

	cout << nazwa << endl;

	inFile.open(nazwa, inFile.in);
	if (!inFile.is_open())
	{
		cout << "Nie bylo mozliwe poprawne otworzenie pliku";
		return 1;
	}
	string s;

	for (int i = 0; i < ZAKRES; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			getline(inFile, s, ',');
		}
		notowania[i] = stod(s);
	}
	return 0;
}

void MACDfunction(double* notowania, double* MACD)
{
	for (int i = 0; i < ZAKRES; i++)
	{
		double d26 = AVGkroczaca(26, i, notowania);
		double d12 = AVGkroczaca(12, i, notowania);
		MACD[i] = d12 - d26;
	}
}

void SIGNALfunction(double* MACD, double* SIGNAL)
{
	for (int i = 0; i < ZAKRES; i++)
	{
		double d9macd = AVGkroczaca(9, i, MACD);
		SIGNAL[i] = d9macd;
	}
}

void Zapisz(const char* pobrane, double* MACD, double* SIGNAL)
{
	FILE* in;
	FILE* out;
	char namein[512] = "notowania/";
	char nameout[512] = "notowania/";
	strcat(namein, pobrane);
	strcat(nameout, pobrane);
	strcat(namein, ".csv");
	strcat(nameout, "_result.csv");

	in = fopen(namein, "r");
	out = fopen(nameout, "w");

	for (int i = 0; i < ZAKRES; i++)
	{
		char result[200];

		fscanf(in, "%s", result);
		fputs(result, out);

		sprintf_s(result, 100, ",%f,%f \n", MACD[i], SIGNAL[i]);
		fputs(result, out);
	}
	fclose(in);
	fclose(out);
}

void Portfel(double* notowania, double* MACD, double* SIGNAL) {
	double kwota = 1000.00; // kwota w portfelu
	double akcje = 0.00; // ilosc akcji posiadanych
	int last_cut = 0; // ostatnie przeciecie wykresow macd i signal
	int opoznienie = 2; // liczba dni opoznionej reakcji po przecieciu
	double kwota_paczki_akcji = 200.0; // kwota wydana podczas jednorazowego kupna
	double czesc_akcji_do_sprzedazy = 0.5; // jaka czesc posiadanych akcji zostanie sprzedana po odpowiednim sygnale
	
	double prev_roznica = 0.0; // poprzednica roznica wartosci macd i signal (z poprzedniego notowania)
	double act_roznica = 0.0; // roznica wartosci macd i signal z obecnego notowania
	
	for (int i = 26; i < ZAKRES; i++) {
		prev_roznica = act_roznica;

		act_roznica = MACD[i] - SIGNAL[i];

		if (prev_roznica < 0.0 && act_roznica > 0.0)
		{
			last_cut = i;
		}
		else if (prev_roznica > 0.0 && act_roznica < 0.0)
		{
			last_cut = i;
		}
		// opoznienie o 2 okresy, sygnal kupna, macd < 0
		if (i - last_cut == opoznienie && act_roznica > 0.0 && MACD[i]<0.0)
		{
			if (kwota >= kwota_paczki_akcji)
			{
				kwota -= kwota_paczki_akcji;
				double kupiono = kwota_paczki_akcji / notowania[i];
				akcje += kupiono;
			}
			else // kup za reszte kapitalu
			{
				double kupiono = kwota / notowania[i];
				kwota = 0.0;
				akcje += kupiono;
			}
			cout << "KUPNO" << endl;
		}
		// opoznienie o 2 okresy, sygnal sprzedazy, macd>0
		if (i - last_cut == opoznienie && act_roznica < 0.0 && MACD[i] > 0.0)
		{
			if (akcje > 0) 
			{
				// sprzedawana jest czesc posiadanych akcji.
				double sprzedano = czesc_akcji_do_sprzedazy *akcje * notowania[i];
				kwota += sprzedano;
				akcje = akcje*(1- czesc_akcji_do_sprzedazy);
				cout << "SPRZEDAZ" << endl;
			}
			
		}

		cout << "NOTOWANIE: " << notowania[i] << "\tKWOTA: " << kwota << "\t AKCJE: " << akcje << "\n";

	}

	kwota += akcje * notowania[ZAKRES - 1]; // sprzedaz reszty akcji
	akcje = 0;

	cout << "KWOTA KONCOWA: " << kwota << endl;
}

int main()
{
	const char* pobrane = "pko";
	
	double* notowania = new double[1000];
	double* MACD_RESULTS = new double[1000];
	double* SIGNAL_RESULTS = new double[1000];

	if (Wczytaj(pobrane, notowania) != 0)
	{
		return 1;
	};
	
	MACDfunction(notowania, MACD_RESULTS);

	SIGNALfunction(MACD_RESULTS, SIGNAL_RESULTS);

	Zapisz(pobrane, MACD_RESULTS, SIGNAL_RESULTS);

	Portfel(notowania, MACD_RESULTS, SIGNAL_RESULTS);

	delete[] SIGNAL_RESULTS;
	delete[] MACD_RESULTS;
	delete[] notowania;
	
	return 0;
}
