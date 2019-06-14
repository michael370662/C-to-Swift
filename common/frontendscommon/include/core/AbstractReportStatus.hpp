#ifndef __PonyTech_Frontend_AbstractReportStatus_hpp__
#define __PonyTech_Frontend_AbstractReportStatus_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)


class AbstractReportStatus
{
public:
    enum ErrorStatus
    {
        es_connected = 0,
        es_disconnected = 1,
        es_general_error = 2,
        es_server_not_reached = 3,
        es_connection_broken  = 4,
        es_version_not_match = 5,
    };

    enum ErrorCode
    {
        ec_none                 = 0,
        ec_genearl_error        = 0x00010001,
        ec_client_socket_error  = 0x00010002,
    };
public:
    AbstractReportStatus(){}
    virtual ~AbstractReportStatus(){}


    virtual void report_status(int status, int code) = 0;
private:
    FND_DISABLE_COPY(AbstractReportStatus);
};



END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_AbstractReportStatus_hpp__
