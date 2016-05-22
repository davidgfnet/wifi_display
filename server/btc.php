<?php

require_once("provider.php");

class BTCExchangeProvider implements ServiceProvider {
	// Widget properties
	static $widgetName = "BTC Exchange";
	static $widgetIcon = "btc.svg";

	public $cpair;
	public $width;
	public $height;

	function BTCExchangeProvider() {
		$this->cpair = "EUR";
		$this->width = 800;
		$this->height = 100;
		$this->font_size = 1;
		$this->font_family = "Verdana";
	}

    public function getTunables() {
		return array(
			"currency"    => array("type" => "text", "display" => "Currency Pair", "value" => $this->cpair),
			"font_family" => array("type" => "text", "display" => "Font Family", "value" => $this->font_family),
			"font_size"   => array("type" => "fnum", "display" => "Font Size", "value" => $this->font_size)
		);
	}
    public function setTunables($v) {
		$this->cpair = strtoupper($v["currency"]["value"]);
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
		// Gather information from fixer.io
		$raw = file_get_contents("https://api.bitcoinaverage.com/ticker/".$this->cpair."/");
		$exchange = json_decode($raw, true);

		$rate = $exchange["24h_avg"];

		// Generate an SVG image out of this 
		return sprintf(
			'<svg width="%d" height="%d" version="1.1" xmlns="http://www.w3.org/2000/svg" 
				xmlns:xlink="http://www.w3.org/1999/xlink">
                <text text-anchor="middle" x="50%%" y="80%%" fill="black" style="font-size: %dpx; font-family: %s;">
					%s/BTC %0.2f
				</text>
			</svg>', $this->width, $this->height,
			    $this->font_size * $this->height, $this->font_family,
				$this->cpair, $rate
		);
	}

};

?>
