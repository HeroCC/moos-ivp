#--------------------------------------------------------
# The CMakeLists.txt for:                       uBackNMEABridge
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  BackNMEABridge.cpp
  BackNMEABridge_Info.cpp
  main.cpp
        Socket.h Socket.cpp
)


ADD_EXECUTABLE(uBackNMEABridge ${SRC})

TARGET_LINK_LIBRARIES(uBackNMEABridge
   ${MOOSGeodesy_LIBRARIES}
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread
   #geometry
   )

set_property(TARGET uBackNMEABridge PROPERTY CXX_STANDARD 11)
