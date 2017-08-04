#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <tlhelp32.h>
#include <psapi.h>
#include <vector>



using namespace std;

bool GetProcID(char const* ProcName);
bool GetDllBase(char const* DllName, DWORD ProcID);
bool GetGameInfo(HANDLE handle, int GameBase, int StormBase);
void WriteGameInfo();


//necessary constants
char const* sProcessName = "war3.exe";
char const* sGameDLL = "Game.dll";
char const* sStormDLL = "Storm.dll";

//variables
DWORD ProcID = 0;
DWORD GameBase = 0;
DWORD StormBase = 0;


//offsets
int nGameStateOffset = 0xACE638;
int nGameRealTimeOffset = 0xAB7E98;


//player names
int nLocalPlayerNameAdress = 0x0;
int nLocalPlayerNameOffset1 = 0xAAE314;
int nLocalPlayerNameOffset2 = 0xC;
int nLocalPlayerNameOffset3 = 0xC;
int nLocalPlayerNameOffset4 = 0x14;
int nLocalPlayerNameOffset5 = 0x8;
int nLocalPlayerNameOffset6 = 0x18;

int nPlayer1NameAdress = 0x0;
int nPlayer1NameOffset1 = 0xAB6E7C;
int nPlayer1NameOffset2 = 0x124;
int nPlayer1NameOffset3 = 0x14;
int nPlayer1NameOffset4 = 0x8;

int nPlayer2NameAdress = 0x0;
int nPlayer2NameOffset1 = 0xAB6E7C;
int nPlayer2NameOffset2 = 0x124;
int nPlayer2NameOffset3 = 0x18;
int nPlayer2NameOffset4 = 0x14;
int nPlayer2NameOffset5 = 0x8;


//resources
int nPxOffset1 = 0x00AB7788;
int nPxOffset2 = 0xC;

int nP1GoldOffset = 0x14;
int nP1WoodOffset = 0x1C;

int nP2GoldOffset = 0x154;
int nP2WoodOffset = 0x15C;

int nP1SupplyOffset = 0x34;
int nP2SupplyOffset = 0x174;

int nPxOffset4 = 0x78;


//max supply
int nPxMaxSupplyOffset1 = 0x4;

int nP1MaxSupplyOffset2 = 0xC;
int nP2MaxSupplyOffset2 = 0xEC;

int nP1MaxSupplyOffset3 = 0x278;
int nP2MaxSupplyOffset3 = 0x7F8;


//races
int nP1RaceOffset1 = 0xAB6EB0;
int nP2RaceOffset1 = 0xAB68A0;


int nP1RaceOffset2 = 0x6AC;
int nP2RaceOffset2 = 0x2A0;



//variables
int nGameState = 0;
int nGameRealTime = 0;
char sLocalPlayerName[20];

char sPlayer1Name[20];
int nP1race;
int nP1gold;
int nP1wood;
int nP1supply;
int nP1maxsupply;

char sPlayer2Name[20];
int nP2race;
int nP2gold;
int nP2wood;
int nP2supply;
int nP2maxsupply;



int main(int argc, const char* argv[]) {

    system("CLS");
    cout << "Looking for WC3..." << endl;
    if (!GetProcID(sProcessName)) {
        return 1;
    }
    cout << "Obtaining " << sGameDLL << " base..." << endl;
    if (!GetDllBase(sGameDLL, ProcID)) {
        return 1;
    }

    cout << "Obtaining " << sStormDLL << " base..." << endl;
    if (!GetDllBase(sStormDLL, ProcID)) {
        return 1;
    }

    HANDLE handle;
    handle = OpenProcess(PROCESS_VM_READ ,false, ProcID);

    cout << endl;


    while(true) {
        GetGameInfo(handle, (int)GameBase, (int)StormBase);
        WriteGameInfo();
        Sleep(1000);
    }

    return 0;

}



bool GetProcID(char const* ProcName) {

    PROCESSENTRY32 pe32;
    HANDLE hSnapshot = NULL;
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    //iterate over process list from snapshot
    if (Process32First(hSnapshot, &pe32)) {
        do {
            //if process was found, break
            if (strcmp(pe32.szExeFile, ProcName ) == 0)
                break;
        }
        while (Process32Next(hSnapshot, &pe32));
    }
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        CloseHandle(hSnapshot);
    }
    //check if we indeed found our process
    if (strcmp(pe32.szExeFile, ProcName) == 0) {
        ProcID = pe32.th32ProcessID;
        cout << "\t" << "found " << sProcessName << " with PID " << ProcID << endl;
        return true;
    }
    else {
        cout << "\t" << sProcessName << " not found" << endl;
        return false;
    }

}



bool GetDllBase(char const* DllName, DWORD ProcID) {
    HANDLE snapMod;
    MODULEENTRY32 me32;
    snapMod = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcID);
    me32.dwSize = sizeof(MODULEENTRY32);
	if (Module32First(snapMod, &me32)) {
        do {
            if (strcmp(DllName,me32.szModule) == 0){
                if (strcmp(DllName, sGameDLL) == 0){
                    GameBase = (DWORD) me32.modBaseAddr;
                    cout << "\t" << "found " << DllName << " at " << "0x" << hex << GameBase << endl;
                    CloseHandle(snapMod);
                    return true;
                }
                if (strcmp(DllName, sStormDLL) == 0){
                    StormBase = (DWORD) me32.modBaseAddr;
                    cout << "\t" << "found " << DllName << " at " << "0x" << hex << StormBase << endl;
                    CloseHandle(snapMod);
                    return true;
                }
            }
        }
        while(Module32Next(snapMod,&me32));
    }
    CloseHandle(snapMod);
    cout << "\t" << "something went wrong - try to check your privileges" << endl;
    return false;

}



bool GetGameInfo(HANDLE handle, int GameBase, int StormBase) {

    //get the gamestate
    ReadProcessMemory (handle, (uint8_t*)GameBase + nGameStateOffset, &nGameState, sizeof (nGameState), NULL);


    //get the local players name
    ReadProcessMemory (handle, (uint8_t*)GameBase + nLocalPlayerNameOffset1, &nLocalPlayerNameAdress, sizeof (nLocalPlayerNameAdress), NULL);
    ReadProcessMemory (handle, (uint8_t*)nLocalPlayerNameAdress + nLocalPlayerNameOffset2, &nLocalPlayerNameAdress, sizeof (nLocalPlayerNameAdress), NULL);
    ReadProcessMemory (handle, (uint8_t*)nLocalPlayerNameAdress + nLocalPlayerNameOffset3, &nLocalPlayerNameAdress, sizeof (nLocalPlayerNameAdress), NULL);
    ReadProcessMemory (handle, (uint8_t*)nLocalPlayerNameAdress + nLocalPlayerNameOffset4, &nLocalPlayerNameAdress, sizeof (nLocalPlayerNameAdress), NULL);
    ReadProcessMemory (handle, (uint8_t*)nLocalPlayerNameAdress + nLocalPlayerNameOffset5, &nLocalPlayerNameAdress, sizeof (nLocalPlayerNameAdress), NULL);
    ReadProcessMemory (handle, (uint8_t*)nLocalPlayerNameAdress + nLocalPlayerNameOffset6, &sLocalPlayerName, sizeof (sLocalPlayerName), NULL);



    //if we are not in game...
    if ((404 > nGameState) or (nGameState > 459)) {
        cout << dec << "\t\t\t\t\t\t\t\t\t  \r" << "Waiting for a game to start, " << "state: " << nGameState << "\r" << flush;
    }
    //if we are in game...
    else {

        //get the games runningtime
        ReadProcessMemory (handle, (uint8_t*)GameBase + nGameRealTimeOffset, &nGameRealTime, sizeof (nGameRealTime), NULL);


        //get player1 name
        ReadProcessMemory (handle, (uint8_t*)GameBase + nPlayer1NameOffset1, &nPlayer1NameAdress, sizeof (nPlayer1NameAdress), NULL);
        ReadProcessMemory (handle, (uint8_t*)nPlayer1NameAdress + nPlayer1NameOffset2, &nPlayer1NameAdress, sizeof (nPlayer1NameAdress), NULL);
        ReadProcessMemory (handle, (uint8_t*)nPlayer1NameAdress + nPlayer1NameOffset3, &nPlayer1NameAdress, sizeof (nPlayer1NameAdress), NULL);
        ReadProcessMemory (handle, (uint8_t*)nPlayer1NameAdress + nPlayer1NameOffset4, &nPlayer1NameAdress, sizeof (nPlayer1NameAdress), NULL);
        ReadProcessMemory (handle, (uint8_t*)nPlayer1NameAdress, &sPlayer1Name, 20, NULL);

        //get player2 name
        ReadProcessMemory (handle, (uint8_t*)GameBase + nPlayer2NameOffset1, &nPlayer2NameAdress, sizeof (nPlayer2NameAdress), NULL);
        ReadProcessMemory (handle, (uint8_t*)nPlayer2NameAdress + nPlayer2NameOffset2, &nPlayer2NameAdress, sizeof (nPlayer2NameAdress), NULL);
        ReadProcessMemory (handle, (uint8_t*)nPlayer2NameAdress + nPlayer2NameOffset3, &nPlayer2NameAdress, sizeof (nPlayer2NameAdress), NULL);
        ReadProcessMemory (handle, (uint8_t*)nPlayer2NameAdress + nPlayer2NameOffset4, &nPlayer2NameAdress, sizeof (nPlayer2NameAdress), NULL);
        ReadProcessMemory (handle, (uint8_t*)nPlayer2NameAdress + nPlayer2NameOffset5, &nPlayer2NameAdress, sizeof (nPlayer2NameAdress), NULL);
        ReadProcessMemory (handle, (uint8_t*)nPlayer2NameAdress, &sPlayer2Name, 20, NULL);


        //read resource info for both players
        int nIntermediateValue = 0;
        ReadProcessMemory (handle, (uint8_t*)GameBase + nPxOffset1, &nIntermediateValue, sizeof (nIntermediateValue), NULL);
        ReadProcessMemory (handle, (uint8_t*)nIntermediateValue + nPxOffset2, &nIntermediateValue, sizeof (nIntermediateValue), NULL);

        //player 1 gold
        ReadProcessMemory (handle, (uint8_t*)nIntermediateValue + nP1GoldOffset, &nP1gold, sizeof (nP1gold), NULL);
        ReadProcessMemory (handle, (uint8_t*)nP1gold + nPxOffset4, &nP1gold, sizeof (nP1gold), NULL);
        //player 1 wood
        ReadProcessMemory (handle, (uint8_t*)nIntermediateValue + nP1WoodOffset, &nP1wood, sizeof (nP1wood), NULL);
        ReadProcessMemory (handle, (uint8_t*)nP1wood + nPxOffset4, &nP1wood, sizeof (nP1wood), NULL);
        //player 1 current supply
        ReadProcessMemory (handle, (uint8_t*)nIntermediateValue + nP1SupplyOffset, &nP1supply, sizeof (nP1supply), NULL);
        ReadProcessMemory (handle, (uint8_t*)nP1supply + nPxOffset4, &nP1supply, sizeof (nP1supply), NULL);

        //player 2 gold
        ReadProcessMemory (handle, (uint8_t*)nIntermediateValue + nP2GoldOffset, &nP2gold, sizeof (nP2gold), NULL);
        ReadProcessMemory (handle, (uint8_t*)nP2gold + nPxOffset4, &nP2gold, sizeof (nP2gold), NULL);
        //player 2 wood
        ReadProcessMemory (handle, (uint8_t*)nIntermediateValue + nP2WoodOffset, &nP2wood, sizeof (nP2wood), NULL);
        ReadProcessMemory (handle, (uint8_t*)nP2wood + nPxOffset4, &nP2wood, sizeof (nP2wood), NULL);
        //player 2 current supply
        ReadProcessMemory (handle, (uint8_t*)nIntermediateValue + nP2SupplyOffset, &nP2supply, sizeof (nP2supply), NULL);
        ReadProcessMemory (handle, (uint8_t*)nP2supply + nPxOffset4, &nP2supply, sizeof (nP2supply), NULL);


        //max supply
        nIntermediateValue = 0;
        ReadProcessMemory (handle, (uint8_t*)GameBase + nPxOffset1, &nIntermediateValue, sizeof (nIntermediateValue), NULL);
        ReadProcessMemory (handle, (uint8_t*)nIntermediateValue + nPxMaxSupplyOffset1, &nIntermediateValue, sizeof (nIntermediateValue), NULL);

        //player 1 max supply
        ReadProcessMemory (handle, (uint8_t*)nIntermediateValue + nP1MaxSupplyOffset2, &nP1maxsupply, sizeof (nP1maxsupply), NULL);
        ReadProcessMemory (handle, (uint8_t*)nP1maxsupply + nP1MaxSupplyOffset3, &nP1maxsupply, sizeof (nP1maxsupply), NULL);
        //make sure we cap at 100
        if (nP1maxsupply > 100) {
                nP1maxsupply = 100;
        }


        //player 2 max supply
        ReadProcessMemory (handle, (uint8_t*)nIntermediateValue + nP2MaxSupplyOffset2, &nP2maxsupply, sizeof (nP2maxsupply), NULL);
        ReadProcessMemory (handle, (uint8_t*)nP2maxsupply + nP2MaxSupplyOffset3, &nP2maxsupply, sizeof (nP2maxsupply), NULL);
        //make sure we cap at 100
        if (nP2maxsupply > 100) {
                 nP2maxsupply = 100;
        }



        //player 1 race
        nIntermediateValue = 0;
        ReadProcessMemory (handle, (uint8_t*)GameBase + nP1RaceOffset1, &nP1race, sizeof (nP1race), NULL);
        ReadProcessMemory (handle, (uint8_t*)nP1race + nP1RaceOffset2, &nP1race, sizeof (nP1race), NULL);

        //player 2 race
        ReadProcessMemory (handle, (uint8_t*)GameBase + nP2RaceOffset1, &nP2race, sizeof (nP2race), NULL);
        ReadProcessMemory (handle, (uint8_t*)nP2race + nP2RaceOffset2, &nP2race, sizeof (nP2race), NULL);



        cout << "\t\t\t\t\t\t\t\t\t  \r";
        cout << dec << sPlayer1Name << "(" << nP1race << ")" << ": g:" << nP1gold/10 << " w:" << nP1wood/10 << " s:" << nP1supply << "/" << nP1maxsupply << "\t"
                    << sPlayer2Name << "(" << nP2race << ")" << ": g:" << nP2gold/10 << " w:" << nP2wood/10 << " s:" << nP2supply << "/" << nP2maxsupply << "\r"
                    << flush;
    }

    return true;
}



void WriteGameInfo() {
    vector<string> vFileNames = {"gameinfo.txt", "ingametime.txt", "localplayer.txt",
    "p1gold.txt", "p1name.txt", "p1race.txt", "p1sup.txt", "p1wood.txt",
    "p2gold.txt", "p2name.txt", "p2race.txt", "p2sup.txt", "p2wood.txt",
    };

    ofstream out ("localplayer.txt");
    out << sLocalPlayerName;
    out.close();

    if ((404 > nGameState) or (nGameState > 459)) {
        for(vector<string>::iterator it = vFileNames.begin(); it != vFileNames.end(); ++it) {
            if (!(*it == "localplayer.txt")) {
                ofstream out(*it);
                out << "";
                out.close();
            }
        }
    }
    else {
        for(vector<string>::iterator it = vFileNames.begin(); it != vFileNames.end(); ++it) {
            if(*it == "gameinfo.txt") {
                ofstream out (*it);
                out << "game info";
                out.close();
            }
            if(*it == "ingametime.txt") {
                int nSeconds = (int)((nGameRealTime/1000) % 60);
                int nMinutes = (int) ((nGameRealTime/(1000*60)) % 60);


                ofstream out (*it);
                out << nMinutes << ":" << nSeconds;
                out.close();
            }

            if(*it == "localplayer.txt") {
                ofstream out (*it);
                out << sLocalPlayerName;
                out.close();
            }

            if(*it == "p1name.txt") {
                ofstream out (*it);
                out << sPlayer1Name;
                out.close();
            }

            if(*it == "p1race.txt") {
                string sP1race;
                switch(nP1race) {
                    case 1: {
                        sP1race = "HU";
                        break;
                    }
                    case 2: {
                        sP1race = "OC";
                        break;
                    }
                    case 3: {
                        sP1race = "UD";
                        break;
                    }
                    case 4: {
                        sP1race = "NE";
                        break;
                    }
                }
                ofstream out (*it);
                out << sP1race;
                out.close();
            }

            if(*it == "p1gold.txt") {
                ofstream out(*it);
                out << "G:" << nP1gold/10;
                out.close();
            }

            if(*it == "p1wood.txt") {
                ofstream out(*it);
                out << "W:" << nP1wood/10;
                out.close();
            }

            if(*it == "p1sup.txt") {
                ofstream out(*it);
                out << "S:" << nP1supply << "/" << nP1maxsupply;
                out.close();
            }

            if(*it == "p2name.txt") {
                ofstream out (*it);
                out << sPlayer2Name;
                out.close();

            }

            if(*it == "p2race.txt") {
                string sP2race;
                switch(nP2race) {
                    case 1: {
                        sP2race = "HU";
                        break;
                    }
                    case 2: {
                        sP2race = "OC";
                        break;
                    }
                    case 3: {
                        sP2race = "UD";
                        break;
                    }
                    case 4: {
                        sP2race = "NE";
                        break;
                    }
                }
                ofstream out (*it);
                out << sP2race;
                out.close();
            }

            if(*it == "p2gold.txt") {
               ofstream out(*it);
               out << "G:" << nP2gold/10;
               out.close();
            }

            if(*it == "p2wood.txt") {
                ofstream out(*it);
                out << "W:" << nP2wood/10;
                out.close();
            }

            if(*it == "p2sup.txt") {
                ofstream out(*it);
                out << "S:" << nP2supply << "/" << nP2maxsupply;
                out.close();
            }
        }
    }

}
