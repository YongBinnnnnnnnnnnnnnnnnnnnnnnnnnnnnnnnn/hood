#!/bin/sh

if which chromium; then
  executable=chromium
  executable_path=/usr/lib/chromium
elif which chrome; then
  executable=chrome
  executable_path=/usr/lib/chrome
elif which chromium-browser; then
  executable=chromium-browser
  executable_path=/usr/lib/chromium-browser
fi
extra_options=

if [ "$1" = "yongbin" ]; then
  extra_options='--user-agent="Mozilla/5.0 (X11; FreeBSD amd64; rv:121.0) Gecko/20100101 Firefox/121.0"'
fi

# I have another script that can randomize useragent, disable script objects like navigator etc.
# The computer used for that script is in a bad condition
# The script could have been stolen but I am going to re-create it.
# Encountered accident while updating this script https://yuba.douyu.com/wb/2469372252391538933

rm -r /tmp/ychrome
mkdir -p /tmp/ychrome
ln -s $executable_path/* /tmp/ychrome
rm /tmp/ychrome/$executable


rm -r ~/.cache/fontconfig/
libfontconfig=$(ldd $executable_path/$executable |grep fontconfig|cut -d " " -f 3)
# -e "s|family\b|------|g"
sed -e "s|default\b|-------|g" -e "s|alias\b|-----|g" -e "s|binding\b|-------|g"  -e "s|prefer\b|------|g" -e "s|/etc|/tmp|g" $libfontconfig >/tmp/ychrome/$(basename $libfontconfig)

#-e "s|hardwareConcurrency\b|$(cat /dev/random | tr -dc "@#%^*_+=0-9A-Za-z"|head -c 19)|g"
#-e "s|getBoundingClientRect\b|$(cat /dev/random | tr -dc "@#%^*_+=/"|head -c 21)|g"
# https://canvasblocker.kkapsner.de/test/detectionTest.html
# https://github.com/EFForg/cover-your-tracks
# https://browserleaks.com/

sed -e "s|fontFamily\b|$(cat /dev/random | tr -dc "@#%^*_+=/"|head -c 10)|g" -e "s|system-font-family\b|$(cat /dev/random | tr -dc "@#%^*_+=/"|head -c 18)|g" -e "s|convertToBlob\b|$(cat /dev/random | tr -dc "@#%^*_+=0-9A-Za-z"|head -c 13)|g" -e "s|getClientRects\b|$(cat /dev/random | tr -dc "@#%^*_+=0-9A-Za-z"|head -c 14)|g" -e "s|toDataURL\b|$(cat /dev/random | tr -dc "@#%^*_+=0-9A-Za-z"|head -c 9)|g" -e "s|getImageData\b|$(cat /dev/random | tr -dc "@#%^*_+=0-9A-Za-z"|head -c 12)|g" $executable_path/$executable > /tmp/ychrome/ychrome
chmod +x /tmp/ychrome/ychrome

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/tmp/ychrome /tmp/ychrome/ychrome --user-data-dir=/tmp/ychrome-data --disable-http2 --block-new-web-contents --disable-background-networking --disable-breakpad --disable-composited-antialiasing --disable-crash-reporter --disable-default-apps --disable-domain-reliability --disable-external-intent-requests --extension-content-verification=enforce_strict --extensions-install-verification=enforce --gpu-rasterization-msaa-sample-count=0 --lite-video-force-override-decision --no-pings --process-per-site --site-per-process --use-fake-device-for-media-stream --disable-features="MediaRouter,WebRTC,SafeBrowsing,msSmartScreenProtection,SendBingSERPTelemetry,SendBingTelemetry,msEdge3PTelemetry,msEdgeCitations,msEdgeFeedbackSupport,msEdgeHJSendBeaconTelemetry,msEdgeHJTelemetry,msEdgeMathHelper,msEdgeQBox,msEdgeSanTelemetry,msEnableMATSTelemetry,msEnableWAMMATSTelemetry,msODSPTelemetry,msOfficeTelemetryBase,msReadAloud,msSendDataDiagnosticTelemetry,msUseSETTelemetryService,AcceptCHFrame,AdInterestGroupAPI,AllowClientHintsToThirdParty,AllowURNsInIframes,AppActivityReporting,AppDiscoveryRemoteUrlSearch,AutofillEnableAccountWalletStorage,AutofillServerCommunication,AutoupgradeMixedContent,BrowsingTopics,ClearCrossSiteCrossBrowsingContextGroupWindowName,ClientHintThirdPartyDelegation,ClientHintsDPR,ClientHintsDPR_DEPRECATED,ClientHintsDeviceMemory,ClientHintsDeviceMemory_DEPRECATED,ClientHintsMetaHTTPEquivAcceptCH,ClientHintsMetaNameAcceptCH,ClientHintsResourceWidth,ClientHintsResourceWidth_DEPRECATED,ClientHintsViewportWidth,ClientHintsViewportWidth_DEPRECATED,ComputePressure,ContextMenuPerformanceInfoAndRemoteHintFetching,ConversionMeasurement,CopyLinkToText,CrashReporting,CriticalClientHint,CrossOriginOpenerPolicyAccessReporting,CrossOriginOpenerPolicyReporting,CrossOriginOpenerPolicyReportingOriginTrial,CrostiniAdditionalEnterpriseReporting,CssSelectorFragmentAnchor,DirectSockets,DocumentReporting,EnableSignedExchangePrefetchCacheForNavigations,EnableSignedExchangeSubresourcePrefetch,EnableStructuredMetrics,EnableSubresourceWebBundles,EnterpriseRealtimeExtensionRequest,EventBasedStatusReporting,ExpectCTReporting,ExperimentalJSProfiler,EnableTLS13EarlyData,FedCm,Fledge,FontAccess,FontAccessPersistent,GreaseUACH,HandwritingRecognitionWebPlatformApiFinch,IdleDetection,InterestGroupStorage,Journeys,LensStandalone,MediaDrmPreprovisioning,MediaEngagementBypassAutoplayPolicies,NTPArticleSuggestions,NetworkTimeServiceQuerying,NotificationTriggers,OmniboxTriggerForNoStatePrefetch,OptimizationHints,OptimizationHintsFetching,OptimizationHintsFetchingAnonymousDataConsent,OptimizationHintsFieldTrials,Parakeet,Prerender2,PreloadMediaEngagementData,ReportAllJavaScriptFrameworks,Reporting,RetailCoupons,SegmentationPlatform,SignedExchangeReportingForDistributors,SpeculationRulesPrefetchProxy,SubresourceWebBundles,TabMetricsLogging,TFLiteLanguageDetectionEnabled,TextFragmentAnchor,SafeBrowsingBetterTelemetryAcrossReports,UACHPlatformEnabledByDefault,UserAgentClientHint,UserAgentClientHintFullVersionList,UsernameFirstFlow,UsernameFirstFlowFilling,UsernameFirstFlowFallbackCrowdsourcing,WebNFC,WebOTP,WebSQLInThirdPartyContextEnabled,WebXR,WinrtGeolocationImplementation,WinrtSensorsImplementation,UserAgentClientHint,ClientHints" --connectivity-check-url=0.0.0.0 --crash-server-url=0.0.0.0 --gcm-checkin-url=0.0.0.0 --gcm-mcs-endpoint=0.0.0.0 --gcm-registration-url=0.0.0.0 --google-apis-url=0.0.0.0 --google-base-url=0.0.0.0 --google-doodle-url=0.0.0.0 --lso-url=0.0.0.0 --oauth-account-manager-url=0.0.0.0 --override-metrics-upload-url=0.0.0.0 --realtime-reporting-url=0.0.0.0 --reporting-connector-url=0.0.0.0 --sync-url=0.0.0.0 --url=0.0.0.0 --variations-server-url=0.0.0.0 --variations-insecure-server-url=0.0.0.0 --bing-base-url=0.0.0.0 --edge-mobile-upsell-service-url=0.0.0.0 --edge-wns-push-server-url=0.0.0.0 --state-label-predictor-model-url=0.0.0.0 --cipher-suite-blacklist="0xcca8,0xc028,0xc027,0xc024,0xc023,0xc014,0xc013,0xc00a,0xc009,0x009d,0x009c,0x0088,0x0087,0x0084,0x006b,0x0067,0x0066,0x0045,0x0044,0x003d,0x003c,0x0039,0x0038,0x0035,0x0033,0x0032,0x002f,0x0027,0x0028,0x0016,0x0013,0x000a,0x0007" --enable-strict-mixed-content-checking --js-flags=--jitless --blink-settings=\"dnsPrefetchingEnabled=false,preferredColorScheme=1,strictMixedContentChecking=true,strictMixedContentCheckingForPlugin=true,strictlyBlockBlockableMixedContent=true\" --disable-explicit-dma-fences --disable-2d-canvas-clip-aa --disable-sync --force-dark-mode --force-webrtc-ip-handling-policy=default_public_interface_only --disable-webgl --disable-webgl2 --disable-signin-scoped-device-id --ssl-version-min=tls1.2 "$extra_options"
# --reduce-user-agent-minor-version --reduce-user-agent-platform-oscpu --reduce-accept-language
