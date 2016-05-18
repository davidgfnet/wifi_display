<?php

interface ServiceProvider {
	// Shape of the object
	// Units go from 0 to 1 (floating point values)
    public function shape();

	// This sets/returns a settings framework
    public function setTunables($v);
    public function getTunables();

	// This generates the widget SVG itself
    public function render();
}

class ProviderAux {
	static function embedSVG($fpath) {
		$rawsvg = base64_encode(file_get_contents($fpath));
		return "data:image/svg+xml;base64,".$rawsvg;
	}
};


?>
