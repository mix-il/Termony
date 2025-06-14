FROM ubuntu:24.04

# Install dependencies
RUN apt-get update && \
    apt install -y build-essential cmake curl wget unzip python3 libncurses-dev \
		git flex bison bash make autoconf libcurl4-openssl-dev tcl \
		gettext zip pigz

ENV OHOS_SDK_RELEASE_VERSION=5.1.0

# Install OHOS SDK
RUN curl -OL https://repo.huaweicloud.com/openharmony/os/${OHOS_SDK_RELEASE_VERSION}-Release/ohos-sdk-windows_linux-public.tar.gz && \
    tar -xvf ohos-sdk-windows_linux-public.tar.gz -C /opt && \
    rm ohos-sdk-windows_linux-public.tar.gz && \
    rm -rf /opt/ohos-sdk/{ohos,windows} && \
    cd /opt/ohos-sdk/linux && \
    for file in $(find . -type f); do \ 
        unzip $file && rm $file; \
    done

# Set environment variables
ENV OHOS_SDK_HOME=/opt/ohos-sdk/linux

CMD /bin/bash
