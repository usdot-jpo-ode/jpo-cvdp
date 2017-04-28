#  This material was prepared by UT-Battelle, LLC (UT-Battelle) under
#  Contract DE-AC05-00OR22725 with the U.S. Department of Energy (DOE). All
#  rights in the material are reserved by DOE on behalf of the Government
#  and UT-Battelle pursuant to the contract. You are authorized to use the
#  material for Government purposes but it is not to be released or
#  distributed to the public. NEITHER THE UNITED STATES NOR THE UNITED
#  STATES DEPARTMENT OF ENERGY, NOR UT-Battelle, NOR ANY OF THEIR EMPLOYEES,
#  MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR
#  RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY
#  INFORMATION, APPARATUS, PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS THAT
#  ITS USE WOULD NOT INFRINGE PRIVATELY OWNED RIGHTS.
# 
#  Created By:
#      Center for Trustworthy Embedded Systems, UT-Battelle
#      
#  Date:
#      Created on Jun 28, 2014
#  
#  Contact:
#      Jason M. Carter, Ph.D.
#      Cyber Security Research Scientist
#      Center for Trustworthy Embedded Systems
#      Cyber and Information Security Research (CISR) Group Oak Ridge National
#      Laboratory
#      865-574-1480 (desk)
#      865-207-2846 (mobile)
#      865-576-5943 (fax)
#      
#      Aaron Ferber
#      Cyber Security Software Engineer
#      Center for Trustworthy Embedded Systems
#      Cyber and Information Security Research (CISR) Group Oak Ridge National
#      Laboratory
#      865-804-5161 (mobile)
cmake_minimum_required(VERSION 2.6)
project(CVDIGeofenceTest)

set(TEST_CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Make a kafka tool to act as producer and consumer.
# This is simulating the ODE.
add_executable(kafka_tool "src/rdkafka_example.cpp")
target_link_libraries(kafka_tool rdkafka++)
