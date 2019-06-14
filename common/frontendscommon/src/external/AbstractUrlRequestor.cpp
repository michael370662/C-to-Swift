#include "external/AbstractUrlRequestor.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

AbstractUrlRequestor::AbstractUrlRequestor()
{
    m_inner = nullptr;
}

AbstractUrlRequestor::~AbstractUrlRequestor()
{
}

void AbstractUrlRequestor::pass_back(const PxConstArray<byte_t>& content, void* userdata)
{
    if (m_inner == nullptr) return;
    m_callback(m_inner, userdata, content.non_const_ptr(), content.count());
}

END_NAMESPACE(Frontend)