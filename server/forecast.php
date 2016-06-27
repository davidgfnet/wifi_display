<?php

require_once("provider.php");

class WeatherForecastProvider implements ServiceProvider {
	// Widget properties
	static $widgetName = "Weather Forecast";
	static $widgetIcon = "forecast.svg";

	public $location;
	public $width;
	public $height;
	public $ndays;

	function WeatherForecastProvider() {
		$this->location = "Barcelona,ES";
		$this->width = 1000;
		$this->height = 200;
		$this->font_size = 0.15;
		$this->font_family = "Arial";
		$this->ndays = 5;
	}

    public function getTunables() {
		return array(
			"location"    => array("type" => "text", "display" => "Location", "value" => $this->location),
			"font_family" => array("type" => "text", "display" => "Font Family", "value" => $this->font_family),
			"font_size"   => array("type" => "fnum", "display" => "Font Size", "value" => $this->font_size),
			"ndays"       => array("type" => "num",  "display" => "Number of days", "value" => $this->ndays)
		);
	}
    public function setTunables($v) {
		$this->location = $v["location"]["value"];
		$this->font_family = $v["font_family"]["value"];
		$this->font_size = $v["font_size"]["value"];
		$this->ndays = $v["ndays"]["value"];
	}

    public function shape() {
		// Return default width/height
		return array(
			"width"       => $this->width,
			"height"      => $this->height,
			"resizable"   => true,
			"keep_aspect" => true,
		);
    }

    public function render() {
		// Gather information from OpenWeatherMap
		$raw = file_get_contents(
			"http://api.openweathermap.org/data/2.5/forecast/daily?cnt=".($this->ndays+1)."&q=".$this->location."&APPID=".GlobalConfig::$weather_api_key
		);
		$weather = json_decode($raw, true);

		$forecast = $weather["list"];

		$daily = array();
		$nd = count($forecast)-1;
		for ($i = 0; $i < $nd; $i++) {
			$icon = $forecast[$i+1]["weather"][0]["icon"];
			$dayn = date('D', $forecast[$i+1]["dt"]);
			$mint = $forecast[$i+1]["temp"]["min"] - 273.15;
			$maxt = $forecast[$i+1]["temp"]["max"] - 273.15;

			$daily[] = sprintf(
				'<image x="%d" y="%d" width="%d" height="%d" xlink:href="%s" />
				<text alignment-baseline="central" text-anchor="middle" x="%d" y="%d" fill="black" style="font-size: %dpx; font-style: %s; font-weight: bold;">
				%s
				</text>
				<text alignment-baseline="central" text-anchor="middle" x="%d" y="%d" fill="black" style="font-size: %dpx; font-style: %s; font-weight: bold;">
				%d° %d°
				</text>
				',
				$this->width / $nd * ($i + 0.05), $this->height * 0.15,
				$this->width / $nd * 0.9, $this->height * 0.6,
				ProviderAux::embedSVG("resources/".$this->imgmap[$icon].".svg"),
				$this->width / $nd * ($i + 0.5), 0.12  * $this->height,
				$this->font_size * $this->height, $this->font_family,
				$dayn,
				$this->width / $nd * ($i + 0.5), 0.92  * $this->height,
				$this->font_size * $this->height, $this->font_family,
				$mint, $maxt
			);
		}

		// Generate an SVG image out of this 
		return sprintf(
			'<svg width="%d" height="%d" version="1.1" xmlns="http://www.w3.org/2000/svg" 
				xmlns:xlink="http://www.w3.org/1999/xlink">
				%s
			</svg>',
				$this->width, $this->height,
				implode("\n", $daily)
		);
	}

	// Weather code -> image mapping!
	public $imgmap = array(
		"01d" => "sun",
		"01n" => "moon",
		"02d" => "sun_cloud",
		"02n" => "moon_cloud",
		"03d" => "sun_scat_cloud",
		"03n" => "moon_scat_cloud",
		"04d" => "clouds",
		"04n" => "clouds",
		"09d" => "shower_rain",
		"09n" => "shower_rain",
		"10d" => "light_rain",
		"10n" => "light_rain",
		"11d" => "thunder",
		"11n" => "thunder",
		"13d" => "snow",
		"13n" => "snow",
		"50d" => "day_mist",
		"50n" => "night_mist",
	);
};

?>
