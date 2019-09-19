<?php

require_once("provider.php");

class PublibikeStationInfoProvider implements ServiceProvider {
	// Widget properties
	static $widgetName = "Publibike";
	static $widgetIcon = "sbb.svg";

	public $station;
	public $width;
	public $height;

	function PublibikeStationInfoProvider() {
		$this->station = 128;  // Binz HB
		$this->width = 400;
		$this->height = 200;
		$this->font_size = 0.65;
		$this->font_family = "Arial";
	}

    public function getTunables() {
		return array(
			"station"    => array("type" => "fnum", "display" => "Station ID", "value" => $this->station),
			"font_family" => array("type" => "text", "display" => "Font Family", "value" => $this->font_family),
			"font_size"   => array("type" => "fnum", "display" => "Font Size", "value" => $this->font_size)
		);
	}
    public function setTunables($v) {
		$this->station = $v["station"]["value"];
		$this->font_family = $v["font_family"]["value"];
		$this->font_size = $v["font_size"]["value"];
	}

    public function shape() {
		// Return default width/height
		return array(
			"width"       => $this->width,
			"height"      => $this->height,
			"resizable"   => true,
			"keep_aspect" => false,
		);
    }

    public function render() {
		// Gather information from OpenWeatherMap
		$raw = file_get_contents("https://api.publibike.ch/v1/public/stations/".$this->station);
		$info = json_decode($raw, true);

		$sname = $info["name"];
		$y = $this->font_size * $this->height;
		$nbike = 0; $nebike = 0;
		for ($i = 0; $i < count($info["vehicles"]); $i++) {
			if ($info["vehicles"][$i]["type"]["id"] == 1)
				$nbike++;
			else
				$nebike++;
		}
		$ret = sprintf('<text x="0" y="%d" fill="black" style="font-size: %dpx; font-style: %s; font-weight: bold;">%s %d/%d</text>', $y, $y, $this->font_family, $sname, $nbike, $nebike);

		// Generate an SVG image out of this 
		return sprintf('<svg width="%d" height="%d" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">%s</svg>',
			$this->width, $this->height, $ret);
	}
};

?>
