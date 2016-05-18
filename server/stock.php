<?php

require_once("provider.php");

class StockProvider implements ServiceProvider {
	// Widget properties
	static $widgetName = "Stock Price";
	static $widgetIcon = "stock.svg";

	public $cpair;
	public $width;
	public $height;

	function StockProvider() {
		$this->stock = "GOOG";
		$this->width = 800;
		$this->height = 100;
		$this->font_size = 1;
		$this->font_family = "Verdana";
	}

    public function getTunables() {
		return array(
			"stock"       => array("type" => "text", "display" => "Stock Name", "value" => $this->stock),
			"font_family" => array("type" => "text", "display" => "Font Family", "value" => $this->font_family),
			"font_size"   => array("type" => "fnum", "display" => "Font Size", "value" => $this->font_size)
		);
	}
    public function setTunables($v) {
		$this->stock = strtoupper($v["stock"]["value"]);
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
		// Gather information from yahoo
		$raw = file_get_contents("http://finance.yahoo.com/webservice/v1/symbols/".$this->stock."/quote?format=json");
		$info = json_decode($raw, true);

		$name  = $info["list"]["resources"][0]["resource"]["fields"]["symbol"];
		$price = $info["list"]["resources"][0]["resource"]["fields"]["price"];

		// Generate an SVG image out of this 
		return sprintf(
			'<svg width="%d" height="%d" version="1.1" xmlns="http://www.w3.org/2000/svg" 
				xmlns:xlink="http://www.w3.org/1999/xlink">
                <text text-anchor="middle" x="50%%" y="80%%" fill="black" style="font-size: %dpx; font-family: %s;">
					%s %0.3f
				</text>
			</svg>', $this->width, $this->height,
			    $this->font_size * $this->height, $this->font_family,
				$name, $price
		);
	}

};

?>
