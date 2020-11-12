#include "./pop_stream.h"


//=============================================================================
template <maniscalco::io::stream_direction S>
maniscalco::io::pop_stream<S>::pop_stream
(
    configuration_type const & configuration
): 
    inputHandler_(configuration.inputHandler_)
{
}



//=============================================================================
namespace maniscalco::io
{
    template class pop_stream<stream_direction::forward>;
    template class pop_stream<stream_direction::reverse>;

} // maniscalco