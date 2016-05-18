<?php

require_once("config.php");
require_once("provider.php");
require_once("weather.php");
require_once("currency.php");
require_once("stock.php");

function array2js($a) {
	if (is_array($a)) {
		$ret = "";
		foreach ($a as $key => $value)
			$ret .= array2js($key).": ".array2js($value).", ";
		return "{".$ret."}";
	}
	else if (is_string($a))
		return "'$a'";

	return $a;
}

class Providers {
	// Returns list of providers
	static function getProvidersList() {
		$providers = array();
		foreach(get_declared_classes() as $klass) {
			$reflect = new ReflectionClass($klass);
			if ($reflect->implementsInterface('ServiceProvider')) {
				$prop = $reflect->getStaticProperties();
				$wname = $prop["widgetName"];
				$providers[$wname] = array("class" => $klass, "icon" => $prop["widgetIcon"]);
			}
		}
		return $providers;
	}

	static function getRender($widget_name, $settings, $ws, $hs) {
		$plist = Providers::getProvidersList();
		$w = new $plist[$widget_name]["class"];
		$w->setTunables($settings);
		if ($ws > 0 && $hs > 0) {
			$w->width = $ws;
			$w->height = $hs;
		}

		return $w->render();
	}
};

// Image rendering stuff

function renderSVG($id) {
	header('Content-type: image/svg+xml');

	// Read the screen and parse it as JSON
	$scr = file_get_contents("screens/".$id);
	$scr = json_decode($scr, true);

	$body = array();
	for ($i = 0; $i < count($scr["widgets"]); $i++) {
		$widget = $scr["widgets"][$i];
		$params = array();
		foreach ($widget["params"] as $p => $v)
			$params[$p] = array("value" => $v);

		$wi = Providers::getRender($widget["type"], $params, $widget["geo"]["w"] * $scr["width"], $widget["geo"]["h"] * $scr["height"]);

		$body[] = sprintf('<image x="%d" y="%d" width="%d" height="%d" xlink:href="%s" />',
		                  $widget["geo"]["x"] * $scr["width"],
		                  $widget["geo"]["y"] * $scr["height"],
		                  $widget["geo"]["w"] * $scr["width"],
		                  $widget["geo"]["h"] * $scr["height"],
		                  "data:image/svg+xml;base64,".base64_encode($wi)
		          );
	}

	$body = implode("\n", $body);

	$svg = sprintf('<svg width="%d" height="%d" version="1.1" xmlns="http://www.w3.org/2000/svg" 
	                  xmlns:xlink="http://www.w3.org/1999/xlink">
	                  %s
	                </svg>', $scr["width"], $scr["height"], $body);

	return array(
		"width"  => $scr["width"],
		"height" => $scr["height"],
		"svg"    => '<?xml version="1.0" encoding="UTF-8" standalone="no"?>'.$svg
	);
}

function renderBMP($id, $numc, $maxwidth, $maxheight) {
	// Render image
	$data = renderSVG($_GET["id"]);
	$svg = $data["svg"];
	// Convert to PNG and scale
	$im = new Imagick();
	$im->readImageBlob($svg);
	$im->setImageFormat("png24");

	// Apply max sizes
	$width = $data["width"];
	$height = $data["height"];
	if ($width > $maxwidth) {
		$ar = $width/$height;
		$width = $maxwidth;
		$height = $width / $ar;
	}
	if ($height > $maxheight) {
		$ar = $width/$height;
		$height = $maxheight;
		$width = $height * $ar;
	}

	$im->resizeImage($width, $height, imagick::FILTER_LANCZOS, 1);

	// Set to gray
	$im->transformImageColorspace(imagick::COLORSPACE_GRAY);
	$im->posterizeImage($numc, imagick::DITHERMETHOD_NO);

	if (isset($_GET["inv"]))
		$im->negateImage(false);

	return $im;
}

?>
