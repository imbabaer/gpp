#pragma once
#include "gep/interfaces/physics.h"

namespace gep
{
    class HavokMeshShape : public IShape
    {
        hkpShape* m_pHkActualShape;
    public:
        HavokMeshShape(hkpShape* havokShape) : m_pHkActualShape(nullptr) { setHkShape(havokShape); }
        ~HavokMeshShape(){ GEP_HK_REMOVE_REF_AND_NULL(m_pHkActualShape); }

        virtual ShapeType::Enum getShapeType() const override { return ShapeType::Triangle; }

        hkpShape* getHkShape() { return m_pHkActualShape; }
        const hkpShape* getHkShape() const { return m_pHkActualShape; }
        void setHkShape(hkpShape* value)
        {
            if (m_pHkActualShape == value) return;
            
            if (m_pHkActualShape)
                m_pHkActualShape->removeReference();
            
            m_pHkActualShape = value;

            if (m_pHkActualShape)
                m_pHkActualShape->addReference();
        }
    };
}
