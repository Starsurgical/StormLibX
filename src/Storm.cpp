#include "Storm.h"


HINSTANCE s_instance;

void StormInitialize() {
}

// @301
void StormDestroyWrapped() {
  //SErrSetBlizzardErrorFunction(nullptr);
  //SDlgDestroy();
  //SGdiDestroy();
  //SVidDestroy();
  SDrawDestroy();
  //SRgnDestroy();  // in Squall
  //SMsgDestroy();
  SNetDestroy();
  //SEvtDestroy();  // in Squall
  //SBltDestroy();
  //SCodeDestroy();
  //SCmdDestroy();
  SFileDestroy();
  //STransDestroy();

  StormDestroy(); // Squall's version
}

// @302
HINSTANCE StormGetInstance() {
  return s_instance;
}
