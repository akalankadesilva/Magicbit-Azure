var utf8=require("utf8");
var crypto=require("crypto");
function createSharedAccessToken(uri, saName, saKey) { 
    if (!uri || !saName || !saKey) { 
           console.log("Usage: node generateSAS.js <IoT Hub Host name> <Device ID> <Shared Access Key>")
           process.exit();
        } 
    var encoded = encodeURIComponent(uri); 
    var now = new Date(); 
    var week = 60*60*24*7;
    var ttl = Math.round(now.getTime() / 1000) + week;
    var signature = encoded + '\n' + ttl; 
    var signatureUTF8 = utf8.encode(signature); 
    var hash = crypto.createHmac('sha256', saKey).update(signatureUTF8).digest('base64'); 
    return 'SharedAccessSignature sr=' + encoded + '&sig=' +  
        encodeURIComponent(hash) + '&se=' + ttl + '&skn=' + saName; 
}
console.log(createSharedAccessToken(process.argv[2],process.argv[3],process.argv[4]));