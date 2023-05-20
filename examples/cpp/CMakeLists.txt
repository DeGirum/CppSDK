add_executable( run_client dg_core_client.cpp )
target_link_libraries( run_client aiclientlib )

add_executable( run_client_async dg_core_client_async.cpp )
target_link_libraries( run_client_async aiclientlib )
