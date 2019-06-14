#include "service/VpnSettingsProvider.hpp"

BEGIN_NAMESPACE(Frontend)

VpnSettingsProvider::VpnSettingsProvider()
{
}

VpnSettingsProvider::~VpnSettingsProvider()
{
}

void VpnSettingsProvider::set(AutoPtr<AbstractVpnSettings> &data)
{
    m_settings.take(data);
}

void VpnSettingsProvider::set_null()
{
    m_settings.release();
}

END_NAMESPACE(Frontend)