export aj_root=/home/xiao/alljoyn

export aj_dist=/home/xiao/alljoyn/core/alljoyn/build/linux/x86_64/debug/dist

export CXXFLAGS="$CXXFLAGS \
			-I$aj_dist/cpp/inc \
			-I$aj_dist/about/inc \
			-I$aj_dist/services_common/inc \
			-I$aj_dist/notification/inc \
			-I$aj_dist/controlpanel/inc \
			-I$aj_dist/services_common/inc \
			-I$aj_dist/samples_common/inc "


export LDFLAGS="$LDFLAGS \
			-L$aj_dist/cpp/lib \
			-L$aj_dist/about/lib \
			-L$aj_dist/services_common/lib \
			-L$aj_dist/notification/lib \
			-L$aj_dist/controlpanel/lib "

export LD_LIBRARY_PATH=/home/xiao/alljoyn/core/alljoyn/build/linux/x86_64/debug/dist/cpp/lib:$LD_LIBRARY_PATH

/home/xiao/alljoyn/core/alljoyn/build/linux/x86_64/debug/dist/cpp/bin/samples/basic_service

