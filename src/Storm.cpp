#include "Storm.h"


HINSTANCE s_instance;

void StormInitialize() {
}

// @301
void StormDestroy() {
  SErrSetBlizzardErrorFunction(nullptr);
  //SDlgDestroy();
  //SGdiDestroy();
  //SVidDestroy();
  SDrawDestroy();
  //SRgnDestroy();
  //SMsgDestroy();
  SNetDestroy();
  //SEvtDestroy();
  //SBltDestroy();
  //SCodeDestroy();
  //SCmdDestroy();
  SFileDestroy();
  //STransDestroy();
}

// @302
HINSTANCE StormGetInstance() {
  return s_instance;
}
