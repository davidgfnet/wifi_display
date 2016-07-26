<?php
  require_once("functions.php");

  function checkUserAuth($adminMode) {
    if (GlobalConfig::$private_ip_only && filter_var($_SERVER["REMOTE_ADDR"], FILTER_VALIDATE_IP, FILTER_FLAG_NO_PRIV_RANGE)) {
       header('HTTP/1.0 401 Unauthorized');
       die('Only local client can connect this server!');
    }
    if ($adminMode && (!isset($_SERVER['PHP_AUTH_USER']) || $_SERVER['PHP_AUTH_PW'] != GlobalConfig::$admin_pass)) {
      header('WWW-Authenticate: Basic realm="Eink admin panel"');
      header('HTTP/1.0 401 Unauthorized');
      die('Need user/pass auth to get here!');
    }
    return true;
  }
?>
