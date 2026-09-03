#include "../Common.h"
#include "../Utility.h"
#include "../Device.h"
#include "Light.h"
using namespace Microsoft::WRL;
using namespace DirectX;
using namespace Ark::Scene;

Light::Light(void)
{
    // ライトとして設定
    setType(ObjectType::eLight);
}

Light::~Light(void)
{

}

void Light::setup(void)
{
    // ライトとして設定
    setType(ObjectType::eLight);
}

void Light::update(void)
{

}

void Light::cleanup(void)
{

}