add_executable(TestVcdParse test_parse_vcd.cxx)
target_link_libraries(TestVcdParse
  sources
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

configure_file(test_kernel.cl test_kernel.cl COPYONLY)

add_executable(TestOpenCL test_opencl.cxx)
target_link_libraries(TestOpenCL
  sources
  OpenCL
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

configure_file(vcd_nand.vcd vcd_nand.vcd COPYONLY)
