/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "Visualization.h"

#include "filesystem/SpecialProtocol.h"
#include "guilib/GUIWindowManager.h"
#include "utils/log.h"

namespace ADDON
{

CVisualization::CVisualization(ADDON::BinaryAddonBasePtr addonBase, float x, float y, float w, float h)
  : IAddonInstanceHandler(ADDON_INSTANCE_VISUALIZATION, addonBase)
{
  // Setup new Visualization instance
  m_name = Name();
  m_presetsPath = CSpecialProtocol::TranslatePath(Path());
  m_profilePath = CSpecialProtocol::TranslatePath(Profile());

  m_struct = {{0}};
  m_struct.props.x = static_cast<int>(x);
  m_struct.props.y = static_cast<int>(y);
  m_struct.props.width = static_cast<int>(w);
  m_struct.props.height = static_cast<int>(h);
  m_struct.props.device = CServiceBroker::GetWinSystem()->GetHWContext();
  m_struct.props.pixelRatio = CServiceBroker::GetWinSystem()->GetGfxContext().GetResInfo().fPixelRatio;
  m_struct.props.name = m_name.c_str();
  m_struct.props.presets = m_presetsPath.c_str();
  m_struct.props.profile = m_profilePath.c_str();
  m_struct.toKodi.kodiInstance = this;
  m_struct.toKodi.transfer_preset = transfer_preset;

  /* Open the class "kodi::addon::CInstanceVisualization" on add-on side */
  if (CreateInstance(&m_struct) != ADDON_STATUS_OK)
  {
    CLog::Log(LOGFATAL, "Visualization: failed to create instance for '%s' and not usable!", ID().c_str());
    return;
  }

  /* presets becomes send with "transfer_preset" during call of function below */
  if (m_struct.toAddon.get_presets)
    m_struct.toAddon.get_presets(&m_struct);
}

CVisualization::~CVisualization()
{
  /* Destroy the class "kodi::addon::CInstanceVisualization" on add-on side */
  DestroyInstance();
}

bool CVisualization::Start(int channels, int samplesPerSec, int bitsPerSample, const std::string& songName)
{
  if (m_struct.toAddon.start)
    return m_struct.toAddon.start(&m_struct, channels, samplesPerSec, bitsPerSample, songName.c_str());
  return false;
}

void CVisualization::Stop()
{
  if (m_struct.toAddon.stop)
    m_struct.toAddon.stop(&m_struct);
}

void CVisualization::AudioData(const float* audioData, int audioDataLength, float *freqData, int freqDataLength)
{
  if (m_struct.toAddon.audio_data)
    m_struct.toAddon.audio_data(&m_struct, audioData, audioDataLength, freqData, freqDataLength);
}

bool CVisualization::IsDirty()
{
  if (m_struct.toAddon.is_dirty)
    return m_struct.toAddon.is_dirty(&m_struct);
  return false;
}

void CVisualization::Render()
{
  if (m_struct.toAddon.render)
    m_struct.toAddon.render(&m_struct);
}

void CVisualization::GetInfo(VIS_INFO *info)
{
  if (m_struct.toAddon.get_info)
    m_struct.toAddon.get_info(&m_struct, info);
}

bool CVisualization::OnAction(VIS_ACTION action, const void *param)
{
  if (m_struct.toAddon.on_action)
    return m_struct.toAddon.on_action(&m_struct, action, param);
  return false;
}

bool CVisualization::HasPresets()
{
  return !m_presets.empty();
}

bool CVisualization::GetPresetList(std::vector<std::string> &vecpresets)
{
  vecpresets = m_presets;
  return !m_presets.empty();
}

int CVisualization::GetActivePreset()
{
  if (m_struct.toAddon.get_active_preset)
    return m_struct.toAddon.get_active_preset(&m_struct);
  return -1;
}

std::string CVisualization::GetActivePresetName()
{
  if (!m_presets.empty())
    return m_presets[GetActivePreset()];
  return "";
}

bool CVisualization::IsLocked()
{
  if (m_struct.toAddon.is_locked)
    return m_struct.toAddon.is_locked(&m_struct);
  return false;
}

void CVisualization::transfer_preset(void* kodiInstance, const char* preset)
{
  CVisualization *addon = static_cast<CVisualization*>(kodiInstance);
  if (!addon || !preset)
  {
    CLog::Log(LOGERROR, "CVisualization::%s - invalid handler data", __FUNCTION__);
    return;
  }

  addon->m_presets.emplace_back(preset);
}

} /* namespace ADDON */
