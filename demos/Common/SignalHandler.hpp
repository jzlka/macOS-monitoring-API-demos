//
//  SignalHandler.hpp
//  
//
//  Created by Jozef on 11/05/2020.
//

#ifndef SignalHandler_hpp
#define SignalHandler_hpp

#include <CoreFoundation/CoreFoundation.h>

void HandleSignalFromRunLoop(CFFileDescriptorRef f, CFOptionFlags callBackTypes, void *info);
void InstallHandleSignalFromRunLoop();

#endif /* SignalHandler_hpp */
