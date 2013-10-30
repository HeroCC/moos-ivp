/****************************************************************/
/*  NAME: Mike Benjamin                                         */
/*  ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA      */
/*  FILE: Sayer_Info.h                                          */
/*  DATE: May 20th, 2013                                        */
/****************************************************************/

#include <cstdlib>
#include <iostream>
#include "Sayer_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

//----------------------------------------------------------------
// Procedure: showSynopsis

void showSynopsis()
{
  blk("SYNOPSIS:                                                       ");
  blk("------------------------------------                            ");
  blk("  The iSay application is used for                              ");
  blk("                                                                ");
  blk("                                                                ");
  blk("                                                                ");
  blk("                                                                ");
}

//----------------------------------------------------------------
// Procedure: showHelpAndExit

void showHelpAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("Usage: iSay file.moos [OPTIONS]                   ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("Options:                                                        ");
  mag("  --alias","=<ProcessName>                                      ");
  blk("      Launch iSay with the given process name other than iSay.  ");
  mag("  --example, -e                                                 ");
  blk("      Display example MOOS configuration block.                 ");
  mag("  --help, -h                                                    ");
  blk("      Display this help message.                                ");
  mag("  --interface, -i                                               ");
  blk("      Display MOOS publications and subscriptions.              ");
  mag("  --version,-v                                                  ");
  blk("      Display the release version of iSay.        ");
  blk("                                                                ");
  blk("Note: If argv[2] does not otherwise match a known option,       ");
  blk("      then it will be interpreted as a run alias. This is       ");
  blk("      to support pAntler launching conventions.                 ");
  blk("                                                                ");
  blk("$ uPokeDB SAY_MOOS=\"say={Would you like to play a game?}, rate=200\" ");
  blk("$ uPokeDB SAY_MOOS=\"say=hal_truth.wav\"                        ");
  blk("                                                                ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showExampleConfigAndExit

void showExampleConfigAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("iSay Example MOOS Configuration                                 ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("ProcessConfig = iSay                                            ");
  blk("{                                                               ");
  blk("  AppTick   = 4                                                 ");
  blk("  CommsTick = 4                                                 ");
  blk("                                                                ");
  blk("  default_voice      = alex       // iSay --voices for others   ");
  blk("  default_rate       = 200                                      ");
  blk("  interval_policy    = from_end   // or from_start              ");
  blk("  min_utter_interval = 1                                        ");
  blk("}                                                               ");
  exit(0);
}


//----------------------------------------------------------------
// Procedure: showInterfaceAndExit

void showInterfaceAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("iSay INTERFACE                                                  ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("SUBSCRIPTIONS:                                                  ");
  blk("------------------------------------                            ");
  blk("  SAY_MOOS = say={Would you like to play a game?}, rate=200     ");
  blk("                                                                ");
  blk("PUBLICATIONS:                                                   ");
  blk("------------------------------------                            ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit()
{
  showReleaseInfo("iSay", "gpl");
  exit(0);
}