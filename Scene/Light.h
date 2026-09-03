#pragma once

// 継承元
#include "BaseObject.h"

/*-------------------------------------------------------------------
*namespace Ark::Scene;
-------------------------------------------------------------------*/
namespace Ark
{
    namespace Scene
    {
        /*-------------------------------------------------------------------
        // 光源
        -------------------------------------------------------------------*/
        class Light : public BaseObject
        {
        public:

            Light(void);
            virtual ~Light(void) override;

            virtual void setup(void) override;
            virtual void update(void) override;
            virtual void cleanup(void) override;


        private:


        };
    }
}