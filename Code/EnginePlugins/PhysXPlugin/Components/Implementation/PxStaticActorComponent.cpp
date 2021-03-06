#include <PhysXPluginPCH.h>

#include <Core/Utils/WorldGeoExtractionUtil.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <PhysXPlugin/Components/PxStaticActorComponent.h>
#include <PhysXPlugin/Utilities/PxConversionUtils.h>
#include <PhysXPlugin/WorldModule/Implementation/PhysX.h>
#include <PhysXPlugin/WorldModule/PhysXWorldModule.h>
#include <extensions/PxRigidActorExt.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezPxStaticActorComponent, 2, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("CollisionMesh", GetMeshFile, SetMeshFile)->AddAttributes(new ezAssetBrowserAttribute("Collision Mesh;Collision Mesh (Convex)")),
    EZ_MEMBER_PROPERTY("CollisionLayer", m_uiCollisionLayer)->AddAttributes(new ezDynamicEnumAttribute("PhysicsCollisionLayer")),
    EZ_MEMBER_PROPERTY("IncludeInNavmesh", m_bIncludeInNavmesh)->AddAttributes(new ezDefaultValueAttribute(true)),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgExtractGeometry, OnMsgExtractGeometry),
  }
  EZ_END_MESSAGEHANDLERS;
  EZ_BEGIN_FUNCTIONS
  {
    EZ_SCRIPT_FUNCTION_PROPERTY(GetShapeId),
  }
  EZ_END_FUNCTIONS;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezPxStaticActorComponent::ezPxStaticActorComponent() = default;
ezPxStaticActorComponent::~ezPxStaticActorComponent() = default;

void ezPxStaticActorComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s << m_hCollisionMesh;
  s << m_uiCollisionLayer;
  s << m_bIncludeInNavmesh;
}


void ezPxStaticActorComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const ezUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();

  s >> m_hCollisionMesh;
  s >> m_uiCollisionLayer;

  if (uiVersion >= 2)
  {
    s >> m_bIncludeInNavmesh;
  }
}

void ezPxStaticActorComponent::OnDeactivated()
{
  if (m_pActor != nullptr)
  {
    ezPhysXWorldModule* pModule = GetWorld()->GetModule<ezPhysXWorldModule>();

    if (pModule->GetPxScene() != nullptr)
    {
      EZ_PX_WRITE_LOCK(*(pModule->GetPxScene()));

      m_pActor->release();
    }

    m_pActor = nullptr;

    pModule->DeleteShapeId(m_uiShapeId);
    pModule->DeallocateUserData(m_uiUserDataIndex);
  }

  SUPER::OnDeactivated();
}

void ezPxStaticActorComponent::OnSimulationStarted()
{
  if (!IsActive())
    return;

  ezPhysXWorldModule* pModule = GetWorld()->GetOrCreateModule<ezPhysXWorldModule>();

  const ezSimdTransform& globalTransform = GetOwner()->GetGlobalTransformSimd();

  PxTransform t = ezPxConversionUtils::ToTransform(globalTransform);
  m_pActor = ezPhysX::GetSingleton()->GetPhysXAPI()->createRigidStatic(t);
  EZ_ASSERT_DEBUG(m_pActor != nullptr, "PhysX actor creation failed");

  ezPxUserData* pUserData = nullptr;
  m_uiUserDataIndex = pModule->AllocateUserData(pUserData);
  pUserData->Init(this);
  m_pActor->userData = pUserData;

  // PhysX does not get any scale value, so to correctly position child objects
  // we have to pretend that this parent object applies no scale on its children
  ezSimdTransform globalTransformNoScale = globalTransform;
  globalTransformNoScale.m_Scale.Set(1.0f);
  AddShapesFromObject(GetOwner(), m_pActor, globalTransformNoScale);

  PxShape* pShape = nullptr;

  if (m_hCollisionMesh.IsValid())
  {
    m_uiShapeId = pModule->CreateShapeId();

    ezResourceLock<ezPxMeshResource> pMesh(m_hCollisionMesh, ezResourceAcquireMode::BlockTillLoaded);

    ezHybridArray<PxMaterial*, 32> pxMaterials;

    {
      const auto& surfaces = pMesh->GetSurfaces();
      pxMaterials.SetCountUninitialized(surfaces.GetCount());

      for (ezUInt32 i = 0; i < surfaces.GetCount(); ++i)
      {
        if (surfaces[i].IsValid())
        {
          ezResourceLock<ezSurfaceResource> pSurface(surfaces[i], ezResourceAcquireMode::BlockTillLoaded);
          if (pSurface)
          {
            pxMaterials[i] = static_cast<PxMaterial*>(pSurface->m_pPhysicsMaterial);

            continue;
          }
        }

        pxMaterials[i] = ezPhysX::GetSingleton()->GetDefaultMaterial();
      }
    }

    if (pxMaterials.IsEmpty())
    {
      pxMaterials.PushBack(ezPhysX::GetSingleton()->GetDefaultMaterial());
    }

    PxMeshScale scale = ezPxConversionUtils::ToScale(globalTransform);

    if (pMesh->GetTriangleMesh() != nullptr)
    {
      pShape = PxRigidActorExt::createExclusiveShape(
        *m_pActor, PxTriangleMeshGeometry(pMesh->GetTriangleMesh(), scale), pxMaterials.GetData(), pxMaterials.GetCount());
    }
    else if (pMesh->GetConvexMesh() != nullptr)
    {
      pShape = PxRigidActorExt::createExclusiveShape(
        *m_pActor, PxConvexMeshGeometry(pMesh->GetConvexMesh(), scale), pxMaterials.GetData(), pxMaterials.GetCount());
    }
    else
    {
      ezLog::Warning("ezPxStaticActorComponent: Collision mesh resource is valid, but it contains no triangle mesh ('{0}' - '{1}')",
        pMesh->GetResourceID(), pMesh->GetResourceDescription());
    }
  }

  if (m_pActor->getNbShapes() == 0)
  {
    ezLog::Error("Static Physics Actor component without shape is used.");
    return;
  }

  if (pShape != nullptr)
  {
    PxFilterData filterData = ezPhysX::CreateFilterData(m_uiCollisionLayer, m_uiShapeId);

    pShape->setSimulationFilterData(filterData);
    pShape->setQueryFilterData(filterData);

    pShape->userData = pUserData;
  }

  {
    EZ_PX_WRITE_LOCK(*(pModule->GetPxScene()));
    pModule->GetPxScene()->addActor(*m_pActor);
  }
}

void ezPxStaticActorComponent::OnMsgExtractGeometry(ezMsgExtractGeometry& msg) const
{
  if (!m_bIncludeInNavmesh)
    return;

  if (msg.m_Mode != ezWorldGeoExtractionUtil::ExtractionMode::CollisionMesh &&
      msg.m_Mode != ezWorldGeoExtractionUtil::ExtractionMode::NavMeshGeneration)
    return;

  if (m_hCollisionMesh.IsValid())
  {
    ezResourceLock<ezPxMeshResource> pMesh(m_hCollisionMesh, ezResourceAcquireMode::BlockTillLoaded);

    pMesh->ExtractGeometry(GetOwner()->GetGlobalTransform(), msg);
  }

  AddShapesToNavMesh(GetOwner(), msg);
}

void ezPxStaticActorComponent::SetMeshFile(const char* szFile)
{
  ezPxMeshResourceHandle hMesh;

  if (!ezStringUtils::IsNullOrEmpty(szFile))
  {
    hMesh = ezResourceManager::LoadResource<ezPxMeshResource>(szFile);
  }

  SetMesh(hMesh);
}

const char* ezPxStaticActorComponent::GetMeshFile() const
{
  if (!m_hCollisionMesh.IsValid())
    return "";

  return m_hCollisionMesh.GetResourceID();
}

void ezPxStaticActorComponent::SetMesh(const ezPxMeshResourceHandle& hMesh)
{
  m_hCollisionMesh = hMesh;
}

EZ_STATICLINK_FILE(PhysXPlugin, PhysXPlugin_Components_Implementation_PxStaticActorComponent);
