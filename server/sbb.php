<?php

require_once("provider.php");

class SBBTimesProvider implements ServiceProvider {
	// Widget properties
	static $widgetName = "SBB";
	static $widgetIcon = "sbb.svg";

	public $station;
	public $numdep;
	public $width;
	public $height;

	function SBBTimesProvider() {
		$this->station = 8503000;  // Zurich HB
		$this->numdep = 5;
		$this->width = 400;
		$this->height = 200;
		$this->font_size = 0.65;
		$this->font_family = "Arial";
	}

    public function getTunables() {
		return array(
			"station"    => array("type" => "fnum", "display" => "Station ID", "value" => $this->station),
			"numdep"    => array("type" => "fnum", "display" => "Board size", "value" => $this->numdep),
			"font_family" => array("type" => "text", "display" => "Font Family", "value" => $this->font_family),
			"font_size"   => array("type" => "fnum", "display" => "Font Size", "value" => $this->font_size)
		);
	}
    public function setTunables($v) {
		$this->station = $v["station"]["value"];
		$this->numdep = $v["numdep"]["value"];
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
		$raw = file_get_contents("http://transport.opendata.ch/v1/stationboard?id=".$this->station);
		$info = json_decode($raw, true);

		$ret = '';
		$y = $this->font_size * $this->height;
		for ($i = 0; $i < $this->numdep; $i++) {
			$zug = $info["stationboard"][$i]["name"];
			$where = $info["stationboard"][$i]["to"];
			$attime = date('G:i', $info["stationboard"][$i]["stop"]["departureTimestamp"]);

			$ret .= sprintf(
				'<text x="%d" y="%d" fill="black" style="font-size: %dpx; font-style: %s; font-weight: bold;">%s %s [%s]</text>',
				0, $y, $this->font_size * $this->height, $this->font_family, $attime, $where, $zug);
			$y += $this->font_size * $this->height;
		}

		// Generate an SVG image out of this 
		return sprintf('<svg width="%d" height="%d" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">%s</svg>',
			$this->width, $this->height, $ret);
	}
};

?>
