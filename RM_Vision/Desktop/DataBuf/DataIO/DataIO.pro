TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

# Add path of header files
INCLUDEPATH += /usr/local/include/opencv4 \
               /usr/local/include/opencv4/opencv2

# Add path of libs
LIBS += /usr/local/lib/libopencv_rgbd.so \
/usr/local/lib/libopencv_xphoto.so \
/usr/local/lib/libopencv_phase_unwrapping.so \
/usr/local/lib/libopencv_imgproc.so \
/usr/local/lib/libopencv_fuzzy.so \
/usr/local/lib/libopencv_aruco.so \
/usr/local/lib/libopencv_datasets.so \
/usr/local/lib/libopencv_img_hash.so \
/usr/local/lib/libopencv_ccalib.so \
/usr/local/lib/libopencv_text.so \
/usr/local/lib/libopencv_core.so \
/usr/local/lib/libopencv_videoio.so \
/usr/local/lib/libopencv_shape.so \
/usr/local/lib/libopencv_xobjdetect.so \
/usr/local/lib/libopencv_highgui.so \
/usr/local/lib/libopencv_face.so \
/usr/local/lib/libopencv_stereo.so \
/usr/local/lib/libopencv_features2d.so \
/usr/local/lib/libopencv_ximgproc.so \
/usr/local/lib/libopencv_dpm.so \
/usr/local/lib/libopencv_optflow.so \
/usr/local/lib/libopencv_line_descriptor.so \
/usr/local/lib/libopencv_quality.so \
/usr/local/lib/libopencv_ml.so \
/usr/local/lib/libopencv_bioinspired.so \
/usr/local/lib/libopencv_surface_matching.so \
/usr/local/lib/libopencv_tracking.so \
/usr/local/lib/libopencv_dnn.so \
/usr/local/lib/libopencv_plot.so \
/usr/local/lib/libopencv_imgcodecs.so \
/usr/local/lib/libopencv_hfs.so \
/usr/local/lib/libopencv_reg.so \
/usr/local/lib/libopencv_dnn_objdetect.so \
/usr/local/lib/libopencv_freetype.so \
/usr/local/lib/libopencv_superres.so \
/usr/local/lib/libopencv_objdetect.so \
/usr/local/lib/libopencv_stitching.so \
/usr/local/lib/libopencv_flann.so \
/usr/local/lib/libopencv_bgsegm.so \
/usr/local/lib/libopencv_photo.so \
/usr/local/lib/libopencv_xfeatures2d.so \
/usr/local/lib/libopencv_saliency.so \
/usr/local/lib/libopencv_video.so \
/usr/local/lib/libopencv_gapi.so \
/usr/local/lib/libopencv_calib3d.so \
/usr/local/lib/libopencv_structured_light.so \
/usr/local/lib/libopencv_videostab.so

# multi threads
unix:!macx: LIBS += -lpthread

SOURCES += main.cpp

HEADERS += \
    ../DataBuf/DataBuf.hpp \
    ../ImageData/ImageData.hpp \
    ../SerialData/SerialData.hpp
