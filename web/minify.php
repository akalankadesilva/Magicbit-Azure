<?php

$file=fopen("admin.html","r");
$dst=fopen("admin.min.html","w");
$content=fread($file,5000);
//$content=str_replace('"','\"' , $content);
$content=str_replace("\n", "",$content);
$content=str_replace("%", "%%",$content);
$content=str_replace("%%s", "%s",$content);
$content=str_replace("\r", "",$content);
$content=str_replace("\t", "",$content);
fwrite($dst,$content);
fclose($file);
fclose($dst);
$file=fopen("restarting.html","r");
$dst=fopen("restarting.min.html","w");
$content=fread($file,5000);
//$content=str_replace('"','\"' , $content);
$content=str_replace("\n", "",$content);
$content=str_replace("%", "%%",$content);
$content=str_replace("%%s", "%s",$content);
$content=str_replace("\r", "",$content);
$content=str_replace("\t", "",$content);
fwrite($dst,$content);
fclose($file);
fclose($dst);
?>