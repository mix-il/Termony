#!/bin/bash
set -e
# build the project on linux
# deveco command line tools is downloaded from:
# https://developer.huawei.com/consumer/cn/download/
# and extracted to any dir
#export TOOL_HOME=""

if [[ ! -n ${TOOL_HOME} ]]; then
  echo """\$TOOL_HOME IS NOT DEFINED, PLS SPECIFIY A CORRECT DIR!
  You can download HarmonyOS Commandline Tools form
  https://developer.huawei.com/consumer/cn/download/
       """
  exit 1
fi

export ARKUIX_VERSION=5.0.1.110
export PROJ_BASE_HOME=$(dirname $(readlink -f "$0"))
export DEVECO_SDK_HOME=$TOOL_HOME/sdk
export PATH=$TOOL_HOME/bin:$PATH
export PATH=$TOOL_HOME/tool/node/bin:$PATH
export PATH=$TOOL_HOME/sdk/default/openharmony/toolchains:$PATH

prepare_arkuix() {
	wget https://repo.huaweicloud.com/arkui-crossplatform/sdk/${ARKUIX_VERSION}/linux/arkui-x-linux-x64-${ARKUIX_VERSION}-Release.zip -c -O ${PROJ_BASE_HOME}/arkuix-sdk.zip
	unzip -o ${PROJ_BASE_HOME}/arkuix-sdk.zip
	# set arkui-x licenses approved
	pushd ${PROJ_BASE_HOME}/arkui-x
		test -d licenses || mkdir licenses
		cp ${PROJ_BASE_HOME}/.ci/arkuix/* licenses/
	popd
        echo "arkui-x.dir=${PROJ_BASE_HOME}/arkui-x" > ${PROJ_BASE_HOME}/local.properties
}

build_harmony_hap() {
	hvigorw assembleHap
	# add hnp, and sign manually
	pushd ${PROJ_BASE_HOME}/entry
		zip -r ../entry/build/default/outputs/default/entry-default-unsigned.hap hnp
	popd
}

build_termony_hnps() {
	cd ${PROJ_BASE_HOME} && make -C build-hnp
}

sign_termony() {
	pushd ${PROJ_BASE_HOME}
		python3 sign.py ./entry/build/default/outputs/default/entry-default-unsigned.hap ./entry/build/default/outputs/default/entry-default-signed.hap
	popd
}


helpusage() {
	echo "Usage: $(basename $0)"
	echo "    -b		Build Termony HNPs and HAP"
	echo "    -s		Sign Termony HAP Package, needed setup Key Signing in DevEco Studio"
	echo "    -p		Push Termony HAP to device"
}

hdc_push() {
	hdc file send ./entry/build/default/outputs/default/entry-default-signed.hap /data/local/tmp
	hdc shell bm install -p /data/local/tmp/entry-default-signed.hap
	hdc shell aa start -a EntryAbility -b $(jq ".app.bundleName" AppScope/app.json5)
}

while getopts ":bsph:" optargs; do
	case ${optargs} in
		b)
			prepare_arkuix
			build_termony_hnps
			build_termony_hap
			;;
		s)
			sign_termony
			;;
		p)
			hdc_push
			;;
		h)
			helpusage
			exit 0
			;;
		:)
			echo -e "  Option doesn't exist: '$OPTARG'"
			helpusage
			;;
	esac
done
