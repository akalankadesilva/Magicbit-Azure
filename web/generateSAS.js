var utf8=require("utf8");
var crypto=require("crypto");
function createSASToken(uri,saName,saKey){
    resourceUri = encodeURIComponent(uri+"/devices/"+saName);

    // Set expiration in seconds
    var expires = (Date.now() / 1000) + 60*60*24*7;
    expires = Math.ceil(expires);
    var toSign = resourceUri + '\n' + expires;

    // Use crypto
    var hmac = crypto.createHmac('sha256', Buffer.from(saKey, 'base64'));
    hmac.update(toSign);
    var base64UriEncoded = encodeURIComponent(hmac.digest('base64'));

    // Construct authorization string
    var token = "SharedAccessSignature sr=" + resourceUri + "&sig="
    + base64UriEncoded + "&se=" + expires;
   
    return token;
}
console.log(createSASToken(process.argv[2],process.argv[3],process.argv[4]));

