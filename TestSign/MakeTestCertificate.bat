MakeCert -r -pe -sv TestCertificate.pvk -n "CN=Deon van der Westhuysen Test Certificate" TestCertificate.cer
pvk2pfx -pvk TestCertificate.pvk -spc TestCertificate.cer -pfx TestCertificate.pfx
