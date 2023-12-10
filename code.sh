#!/bin/sh


if [ "$1" = "stage2" ]; then
  echo  $(whoami)
  script_dir=$(readlink -f $(dirname "$0"))
  #sudo mount -o bind $script_dir/proxy/resolv.conf /etc/resolv.conf
  cat /etc/resolv.conf
  env_dir=$(readlink -f $(dirname "$0"))/environment
  mkdir -p "$env_dir/code/extensions" "$env_dir/code/data"
  code_exe=codium
  code_path=/usr/share/codium
  if test -f /usr/local/share/code-oss/code-oss; then
    code_exe=code-oss
    code_path=/usr/local/share/code-oss
  elif test -f /usr/share/code/code; then
    code_exe=code
    code_path=/usr/share/code
  fi
  rm -r /tmp/code
  cp -r $code_path /tmp/code
  mv /tmp/code/$code_exe /tmp/code/ycode
  sed -i "s|...../dns-query|123456.nxdomain|g" /tmp/code/ycode
  echo $2
  /tmp/code/ycode --ms-enable-electron-run-as-node  --block-new-web-contents --disable-background-networking --disable-breakpad --disable-composited-antialiasing --disable-crash-reporter --disable-default-apps --disable-domain-reliability --disable-external-intent-requests --extension-content-verification=enforce_strict --extensions-install-verification=enforce --gpu-rasterization-msaa-sample-count=0 --lite-video-force-override-decision --no-pings --process-per-site --site-per-process --use-fake-device-for-media-stream --disable-features="WebRTC,SafeBrowsing,msSmartScreenProtection,SendBingSERPTelemetry,SendBingTelemetry,msEdge3PTelemetry,msEdgeCitations,msEdgeFeedbackSupport,msEdgeHJSendBeaconTelemetry,msEdgeHJTelemetry,msEdgeMathHelper,msEdgeQBox,msEdgeSanTelemetry,msEnableMATSTelemetry,msEnableWAMMATSTelemetry,msODSPTelemetry,msOfficeTelemetryBase,msReadAloud,msSendDataDiagnosticTelemetry,msUseSETTelemetryService,AcceptCHFrame,AdInterestGroupAPI,AllowClientHintsToThirdParty,AllowURNsInIframes,AppActivityReporting,AppDiscoveryRemoteUrlSearch,AutofillEnableAccountWalletStorage,AutofillServerCommunication,AutoupgradeMixedContent,BrowsingTopics,ClearCrossSiteCrossBrowsingContextGroupWindowName,ClientHintThirdPartyDelegation,ClientHintsDPR,ClientHintsDPR_DEPRECATED,ClientHintsDeviceMemory,ClientHintsDeviceMemory_DEPRECATED,ClientHintsMetaHTTPEquivAcceptCH,ClientHintsMetaNameAcceptCH,ClientHintsResourceWidth,ClientHintsResourceWidth_DEPRECATED,ClientHintsViewportWidth,ClientHintsViewportWidth_DEPRECATED,ComputePressure,ContextMenuPerformanceInfoAndRemoteHintFetching,ConversionMeasurement,CopyLinkToText,CrashReporting,CriticalClientHint,CrossOriginOpenerPolicyAccessReporting,CrossOriginOpenerPolicyReporting,CrossOriginOpenerPolicyReportingOriginTrial,CrostiniAdditionalEnterpriseReporting,CssSelectorFragmentAnchor,DirectSockets,DocumentReporting,EnableSignedExchangePrefetchCacheForNavigations,EnableSignedExchangeSubresourcePrefetch,EnableStructuredMetrics,EnableSubresourceWebBundles,EnterpriseRealtimeExtensionRequest,EventBasedStatusReporting,ExpectCTReporting,ExperimentalJSProfiler,EnableTLS13EarlyData,FedCm,Fledge,FontAccess,FontAccessPersistent,GreaseUACH,HandwritingRecognitionWebPlatformApiFinch,IdleDetection,InterestGroupStorage,Journeys,LensStandalone,MediaDrmPreprovisioning,MediaEngagementBypassAutoplayPolicies,NTPArticleSuggestions,NetworkTimeServiceQuerying,NotificationTriggers,OmniboxTriggerForNoStatePrefetch,OptimizationHints,OptimizationHintsFetching,OptimizationHintsFetchingAnonymousDataConsent,OptimizationHintsFieldTrials,Parakeet,Prerender2,PreloadMediaEngagementData,ReportAllJavaScriptFrameworks,Reporting,RetailCoupons,SegmentationPlatform,SignedExchangeReportingForDistributors,SpeculationRulesPrefetchProxy,SubresourceWebBundles,TabMetricsLogging,TFLiteLanguageDetectionEnabled,TextFragmentAnchor,SafeBrowsingBetterTelemetryAcrossReports,UACHPlatformEnabledByDefault,UserAgentClientHint,UserAgentClientHintFullVersionList,UsernameFirstFlow,UsernameFirstFlowFilling,UsernameFirstFlowFallbackCrowdsourcing,WebNFC,WebOTP,WebSQLInThirdPartyContextEnabled,WebXR,WinrtGeolocationImplementation,WinrtSensorsImplementation,UserAgentClientHint,ClientHints" --connectivity-check-url=0.0.0.0 --crash-server-url=0.0.0.0 --gcm-checkin-url=0.0.0.0 --gcm-mcs-endpoint=0.0.0.0 --gcm-registration-url=0.0.0.0 --google-apis-url=0.0.0.0 --google-base-url=0.0.0.0 --google-doodle-url=0.0.0.0 --lso-url=0.0.0.0 --oauth-account-manager-url=0.0.0.0 --override-metrics-upload-url=0.0.0.0 --realtime-reporting-url=0.0.0.0 --reporting-connector-url=0.0.0.0 --sync-url=0.0.0.0 --url=0.0.0.0 --variations-server-url=0.0.0.0 --variations-insecure-server-url=0.0.0.0 --bing-base-url=0.0.0.0 --edge-mobile-upsell-service-url=0.0.0.0 --edge-wns-push-server-url=0.0.0.0 --state-label-predictor-model-url=0.0.0.0 --cipher-suite-blacklist="0xc028,0xc027,0xc024,0xc023,0xc014,0xc013,0xc00a,0xc009,0x009d,0x009c,0x0088,0x0087,0x0084,0x006b,0x0067,0x0066,0x0045,0x0044,0x003d,0x003c,0x0039,0x0038,0x0035,0x0033,0x0032,0x002f,0x0027,0x0028,0x0016,0x0013,0x000a,0x0007" --enable-strict-mixed-content-checking --js-flags=--jitless --blink-settings=\"dnsPrefetchingEnabled=false,preferredColorScheme=1,strictMixedContentChecking=true,strictMixedContentCheckingForPlugin=true,strictlyBlockBlockableMixedContent=true\" --disable-explicit-dma-fences --disable-2d-canvas-clip-aa --disable-sync --force-dark-mode --force-webrtc-ip-handling-policy=default_public_interface_only --disable-webgl --disable-webgl2 --disable-signin-scoped-device-id --ssl-version-min=tls1.2 $2 .
  #  --disable-gpu --extensions-dir "$env_dir/code/extensions" --user-data-dir "$env_dir/code/data"
  exit
elif [ "$1" = "stage1" ]; then
  sudo -u $2 $0 stage2 $3
  exit
fi

cat /etc/resolv.conf
sudo unshare -m $0 stage1 $(whoami) $1

