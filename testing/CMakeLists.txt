cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(testing)
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/components/nimble)
