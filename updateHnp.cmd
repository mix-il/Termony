@echo off
set passwd=
DEL hnp\arm64-v8a\*.hnp
COPY C:%HOMEPATH%\Desktop\*.hnp hnp\arm64-v8a\
COPY entry\build\default\outputs\default\entry-default-unsigned.hap %TMP%\unsigned.hap
"C:\Program Files\7-zip\7z" a %TMP%\unsigned.hap hnp
java -jar "C:\Program Files\Huawei\DevEco Studio\sdk\default\openharmony\toolchains\lib\hap-sign-tool.jar" ^
    sign-app ^
    -keyAlias debugKey ^
    -signAlg SHA256withECDSA ^
    -mode localSign ^
    -appCertFile C:%HOMEPATH%/.ohos/config/default_Termony_*.cer ^
    -profileFile C:%HOMEPATH%/.ohos/config/default_Termony_*.p7b ^
    -keystoreFile C:%HOMEPATH%/.ohos/config/default_Termony_*.p12 ^
    -keyPwd %passwd% -keystorePwd %passwd% ^
    -inFile %TMP%\unsigned.hap -outFile C:%HOMEPATH%\Desktop\signed.hap
