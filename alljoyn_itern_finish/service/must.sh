export aj_root=~/alljoyn

export aj_dist=$aj_root/core/alljoyn/build/linux/x86_64/debug/dist

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

ls /home/xiao
