//
//  IOKit_driver_demo.hpp
//  IOKit-driver demo
//
//  Created by Jozef on 28/05/2019.
//  Copyright © 2020 Jozef Zuzelka. All rights reserved.
//

#include <IOKit/IOService.h>
class com_jzlka_driver_IOKit_demo : public IOService
{
OSDeclareDefaultStructors(com_jzlka_driver_IOKit_demo)
public:
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    virtual IOService *probe(IOService *provider, SInt32 *score) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
};
