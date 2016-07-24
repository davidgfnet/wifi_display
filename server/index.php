<?php

require_once("functions.php");

// Make sure it's private network only
checkUserAuth(false);

function mapColor($color, $numc) {
	return intval($color / intval(256/$numc));
}

$numc = isset($_GET["nc"]) ? $_GET["nc"] : 4;

// Render a screen
if (isset($_GET["action"]) && isset($_GET["id"]) && $_GET["action"] == "render") {
    header('Content-type: image/svg+xml');
	die(renderSVG($_GET["id"])["svg"]);
}
// Render a screen
if (isset($_GET["action"]) && isset($_GET["id"]) && $_GET["action"] == "renderpng") {
	$im = renderBMP($_GET["id"], $numc, 0, 0);

    header('Content-type: image/png');
	die($im);
}

// Render a screen
if (isset($_GET["action"]) && isset($_GET["id"]) && $_GET["action"] == "rendereink") {
	$im = renderBMP($_GET["id"], $numc, 0, 0);

    header('Content-type: application/octet-stream');
    header('Content-Length: 61440');

	// Calculate the bit packing for the number of colors.
	$numbits = intval(log10($numc)/log10(2));
	$nppb = 8 / $numbits; // Number of pixels per byte!

	$bppimage = array();
	$it = $im->getPixelIterator();
	foreach ($it as $row => $pixels) {
		$orow = array();
		foreach ($pixels as $column => $pixel) {
			$color = $pixel->getColor()["r"];
			$orow[] = mapColor($color, $numc);
		}
		$it->syncIterator();

		// Process output row to pack colors!
		for ($i = 0; $i < $im->getImageWidth(); $i += $nppb) {
			$color = 0;
			for ($j = 0; $j < $nppb; $j++)
				$color = ($color << $numbits) | $orow[$i+$j];
			//echo chr($color);
			$bppimage[] = $color;
		}
	}

	// Compress image!
	$bppimage = img_compress($bppimage);
	foreach ($bppimage as $b)
		echo chr($b);

	die();
}

?>
