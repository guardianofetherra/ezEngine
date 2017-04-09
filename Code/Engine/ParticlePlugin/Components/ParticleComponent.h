#pragma once

#include <ParticlePlugin/Basics.h>
#include <Core/World/World.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <Core/ResourceManager/ResourceHandle.h>
#include <Core/ResourceManager/ResourceBase.h>
#include <Core/World/ComponentManager.h>
#include <RendererCore/Components/RenderComponent.h>
#include <ParticlePlugin/Effect/ParticleEffectController.h>

class ezParticleRenderData;
struct ezUpdateLocalBoundsMessage;
struct ezExtractRenderDataMessage;
class ezParticleSystemInstance;
class ezParticleComponent;

typedef ezTypedResourceHandle<class ezParticleEffectResource> ezParticleEffectResourceHandle;

typedef ezComponentManagerSimple<ezParticleComponent, ezComponentUpdateType::WhenSimulating> ezParticleComponentManager;

/// \brief This message makes an ezParticleComponent start or stop it's effect.
struct ezParticleComponent_PlayEffectMsg : public ezScriptFunctionMessage
{
  EZ_DECLARE_MESSAGE_TYPE(ezParticleComponent_PlayEffectMsg, ezScriptFunctionMessage);

  /// If true, StartEffect() is called, otherwise StopEffect() is called.
  bool m_bPlay = true;
};

class EZ_PARTICLEPLUGIN_DLL ezParticleComponent : public ezRenderComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezParticleComponent, ezRenderComponent, ezParticleComponentManager);

public:
  ezParticleComponent();
  ~ezParticleComponent();

  void Update();

  virtual void SerializeComponent(ezWorldWriter& stream) const override;
  virtual void DeserializeComponent(ezWorldReader& stream) override;

  //////////////////////////////////////////////////////////////////////////
  // Interface

  /// \brief Starts a new particle effect. If one is already running, it will be stopped (but not interrupted) and a new one is started as well.
  ///
  /// Returns false, if no valid particle resource is specified.
  bool StartEffect();
  /// \brief Stops emitting further particles, making any existing particle system stop in a finite amount of time.
  void StopEffect();
  /// \brief Cancels the entire effect immediately, it will pop out of existence.
  void InterruptEffect();
  /// \brief Returns true, if an effect is currently in a state where it might emit new particles
  bool IsEffectActive() const;

  /// Message Handler for ezParticleComponent_PlayEffectMsg
  void Play(ezParticleComponent_PlayEffectMsg& msg);

  //////////////////////////////////////////////////////////////////////////
  // Properties

  void SetParticleEffect(const ezParticleEffectResourceHandle& hEffect);
  EZ_FORCE_INLINE const ezParticleEffectResourceHandle& GetParticleEffect() const { return m_hEffectResource; }

  void SetParticleEffectFile(const char* szFile);
  const char* GetParticleEffectFile() const;

  ezUInt64 m_uiRandomSeed;
  ezString m_sSharedInstanceName;

  bool m_bSpawnAtStart;
  bool m_bAutoRestart;
  ezTime m_MinRestartDelay;
  ezTime m_RestartDelayRange;

  //////////////////////////////////////////////////////////////////////////


  virtual ezResult GetLocalBounds(ezBoundingBoxSphere& bounds, bool& bAlwaysVisible) override;

  ezParticleEffectController m_EffectController;

protected:
  ezParticleEffectResourceHandle m_hEffectResource;
  ezTime m_RestartTime;

  virtual void OnDeactivated() override;

};
