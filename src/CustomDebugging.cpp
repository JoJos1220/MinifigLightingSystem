/*************************MinifigLightningSystem Project by Jojo1220**********************************
  Filename:     CustomDebugging.cpp
  Purpose:      See CustomDebugging.h
  Autor:        Jojo1220
  Created:      24.04.2023
  Modified:     21.09.2023
  Lizenz:       CC BY-NC-ND 4.0
  Notification: See CustomDebugging.h
******************************************v1.0.0****************************************************/

#include <CustomDebugging.h>

/*-----------------------------------------------------------------------------
 * ONLY for Debugging Purpose! Function to Print significant Parameters
 * ----------------------------------------------------------------------------
*/
void ESPDebuggingFunction(void){
  DEBUG_SERIAL.print("Free heap: ");
  DEBUG_SERIAL.print(ESP.getFreeHeap());
  DEBUG_SERIAL.print("\tFragmentation Metric [%]: ");
  DEBUG_SERIAL.print(ESP.getHeapFragmentation());
  DEBUG_SERIAL.print("\tMax free Block Size in RAM: ");
  DEBUG_SERIAL.println(ESP.getMaxFreeBlockSize());
}