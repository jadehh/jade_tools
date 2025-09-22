SET(OPENCV_DIR "" CACHE PATH "Location of libraries")
if (NOT DEFINED OPENCV_DIR OR OPENCV_DIR STREQUAL "")
    message("-- Find OPENCV_DIR failed,Please set OPENCV_DIR with -D OPENCV_DIR=/usr/local/opencv")
else()
    message("-- Find OPENCV_DIR success:${OPENCV_DIR}")
    if (NOT WIN32)
        set(OPENCV_INCLUDE_DIR "${OPENCV_DIR}/include/opencv4")
        set(OPENCV_LIB_DIR "${OPENCV_DIR}/lib")
        set(OPENCV_LIBS
                opencv_calib3d
                opencv_core
                opencv_dnn
                opencv_features2d
                opencv_flann
                opencv_highgui
                opencv_imgcodecs
                opencv_imgproc
                opencv_ml
                opencv_objdetect
                opencv_photo
                opencv_stitching
                opencv_video
                opencv_videoio
                opencv_cudacodec
        )
    else ()
        set(OPENCV_INCLUDE_DIR "${OPENCV_DIR}/${TARGET_ARCHITECTURE}/include/")
        set(OPENCV_LIB_DIR "${OPENCV_DIR}/${TARGET_ARCHITECTURE}/${TARGET_ARCHITECTURE}/vc15/lib")
        if (TARGET_ARCHITECTURE STREQUAL "x86")
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(OPENCV_LIBS opencv_world340d)
            else ()
                set(OPENCV_LIBS opencv_world340)
            endif ()
        else ()
            set(OPENCV_LIBS
                    opencv_calib3d4120
                    opencv_core4120
                    opencv_dnn4120
                    opencv_features2d4120
                    opencv_flann4120
                    opencv_highgui4120
                    opencv_imgcodecs4120
                    opencv_imgproc4120
                    opencv_ml4120
                    opencv_objdetect4120
                    opencv_photo4120
                    opencv_stitching4120
                    opencv_video4120
                    opencv_videoio4120)
        endif ()
    endif ()
    include_directories(${OPENCV_INCLUDE_DIR})
    link_directories(${OPENCV_LIB_DIR})
endif ()
