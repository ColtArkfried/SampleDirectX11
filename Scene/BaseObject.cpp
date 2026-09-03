#include "../Utility.h"
#include "../Display.h"
#include "BaseObject.h"
using namespace DirectX;
using namespace Ark::Scene;

//-------------------------------------------------------------------
// *コンストラクタ・デストラクタ
BaseObject::BaseObject(void)
{
    mPosition    = XMFLOAT3(0.0f, 0.0f, 0.0f);
    mType        = ObjectType::eNullObject;
    mProcessFlag = true;
    mDeleteFlag  = false;
}

BaseObject::~BaseObject(void)
{

}

BaseObject::BaseObject(const BaseObject& copy)
{
    mPosition    = copy.getPosition();
    mType        = copy.getType();
    mProcessFlag = copy.getProcessFlag();
    mDeleteFlag  = false;
}

const BaseObject& BaseObject::operator = (const BaseObject& copy)
{
    mPosition    = copy.getPosition();
    mType        = copy.getType();
    mProcessFlag = copy.getProcessFlag();
    mDeleteFlag  = false;

    return copy;
}