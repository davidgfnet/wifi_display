<?php
  require_once("functions.php");

  checkUserAuth(true);

  // Save screen!
  if (isset($_GET["action"]) && isset($_GET["id"]) && $_GET["action"] == "save") {
    // Just write post data to file
    file_put_contents("screens/".$_GET["id"], file_get_contents("php://input"));
    die("Saved");
  }

  // Render a widget
  if (isset($_GET["action"]) && $_GET["action"] == "preview") {
    header('Content-type: image/svg+xml');
    die(Providers::getRender($_GET["widget"], json_decode($_GET["tuns"], true), 0, 0));
  }

?>
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->
    <meta name="description" content="">
    <meta name="author" content="">
    <link rel="icon" href="favicon.ico">

    <title>Off Canvas Template for Bootstrap</title>

    <!-- Bootstrap core CSS -->
    <link href="css/bootstrap.min.css" rel="stylesheet">

    <!-- IE10 viewport hack for Surface/desktop Windows 8 bug -->
    <link href="css/ie10-viewport-bug-workaround.css" rel="stylesheet">

    <!-- Custom styles for this template -->
    <link href="css/offcanvas.css" rel="stylesheet">

    <!-- Just for debugging purposes. Don't actually copy these 2 lines! -->
    <!--[if lt IE 9]><script src="js/ie8-responsive-file-warning.js"></script><![endif]-->
    <script src="js/ie-emulation-modes-warning.js"></script>

    <!-- HTML5 shim and Respond.js for IE8 support of HTML5 elements and media queries -->
    <!--[if lt IE 9]>
      <script src="https://oss.maxcdn.com/html5shiv/3.7.2/html5shiv.min.js"></script>
      <script src="https://oss.maxcdn.com/respond/1.4.2/respond.min.js"></script>
    <![endif]-->

    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>
    <script src="//code.interactjs.io/v1.2.6/interact.min.js"></script>

    <script type="text/javascript">

    // Aux functions
    function clone(obj) {
      if (null == obj || "object" != typeof obj) return obj;
      var copy = obj.constructor();
      for (var attr in obj) {
        if (obj.hasOwnProperty(attr)) copy[attr] = clone(obj[attr]);
      }
      return copy;
    }

    function dragMoveListener (event) {
      var target = event.target,
          // keep the dragged position in the data-x/data-y attributes
          x = (parseFloat(target.getAttribute('data-x')) || 0) + event.dx,
          y = (parseFloat(target.getAttribute('data-y')) || 0) + event.dy;

      // translate the element
      target.style.webkitTransform =
      target.style.transform =
        'translate(' + x + 'px, ' + y + 'px)';

      // update the posiion attributes
      target.setAttribute('data-x', x);
      target.setAttribute('data-y', y);
    }

    interact('.resize-drag')
      .draggable({
        onmove: function (event) {
          window.dragMoveListener(event);
        }
      })
      .resizable({
        preserveAspectRatio: true,
        edges: { left: false, right: true, bottom: true, top: false }
      })
      .on('doubletap', function (event) {
        $("#tunables_modal_body").empty();
        var target = event.target.getAttribute("data-i");
        var warr = instance_array[target]["tunables"];
        for (var idx in warr) {
          var nlabel = jQuery('<label/>', {
            for: "tunin_" + idx,
            text: warr[idx]["display"],
          });
          var ninput = jQuery('<input/>', {
            id: "tunin_" + idx,
            name: "tunin_" + idx,
            type: 'text',
            class: 'form-control input-lg',
            value: warr[idx]["value"],
          });
          var ndiv = jQuery('<div/>', {
            id: "div_"+idx,
            class: 'form-group',
          });
          nlabel.appendTo(ndiv);
          ninput.appendTo(ndiv);
          ndiv.appendTo($("#tunables_modal_body"));
        }

        event.preventDefault();
        $('#tunables_modal').modal('show')
        edit_object = target;
      })
      .on('resizemove', function (event) {
        var target = event.target,
            x = (parseFloat(target.getAttribute('data-x')) || 0),
            y = (parseFloat(target.getAttribute('data-y')) || 0);

        // update the element's style
        if (event.rect.width < 16 || event.rect.height < 16) return;
        if (event.rect.width > $("#playground").width() && event.rect.height > $("#playground").height()) return;

        target.style.width  = event.rect.width + 'px';
        target.style.height = event.rect.height + 'px';

        // translate when resizing from top or left edges
        x += event.deltaRect.left;
        y += event.deltaRect.top;

        target.style.webkitTransform = target.style.transform =
            'translate(' + x + 'px,' + y + 'px)';

        target.setAttribute('data-x', x);
        target.setAttribute('data-y', y);
        target.setAttribute('data-w', event.rect.width);
        target.setAttribute('data-h', event.rect.height);
      });

      var prev_w;
      function pground_resize(objresize) {
        // Update height accordingly
        var w = $("#screen_width").val();
        var h = $("#screen_height").val();
        var r = w/h;

        $("#playground").height($("#playground").width() / r);

        // Now move and scale all object accordingly as well :D
        if (objresize) {
          var sfactor = $("#playground").width() / prev_w;

          $(".resize-drag").each(function (idx) {
            var target = $(this).get(0);
            console.log(target);
            console.log(target.getAttribute('data-x'));
            var x = (parseFloat(target.getAttribute('data-x')) || 0) * sfactor;
            var y = (parseFloat(target.getAttribute('data-y')) || 0) * sfactor;
            var w = (parseFloat(target.getAttribute('data-w')) || 100) * sfactor;
            var h = (parseFloat(target.getAttribute('data-h')) || 100) * sfactor;

            target.style.webkitTransform = target.style.transform =
              'translate(' + x + 'px,' + y + 'px)';
            target.style.width  = w + 'px';
            target.style.height = h + 'px';

            target.setAttribute('data-x', x);
            target.setAttribute('data-y', y);
            target.setAttribute('data-w', w);
            target.setAttribute('data-h', h);
          });
        }

        prev_w = $("#playground").width();
      }

      var pg_width = 0, pg_height = 0;
      $(window).resize(function() {
        if (pg_width != $("#playground").width() || pg_height != $("#playground").height()) {
          pground_resize(true);
        }
        pg_width = $("#playground").width();
        pg_height = $("#playground").height();
      });

      var edit_object = null;
      $(document).ready(function() {
        $("#screen_height").change(function () { pground_resize(false); });
        $("#screen_width").change(function () { pground_resize(false); });
        prev_w = $("#playground").width();
        pground_resize(false);

        // Modal save!
        $('#modal_save').click(function(){
          // Save properties!
          var tuns = instance_array[edit_object]["tunables"];
          for (var tun in tuns) {
            tuns[tun]["value"] = $("#tunin_" + tun).val();
          }
          console.log(tuns);

          reloadImages();
          $('#tunables_modal').modal('hide');
        });

        // Widget delete!
        $('#modal_del').click(function(){
          $('[data-i^='+edit_object + ']').remove();
          delete instance_array[edit_object];
          $('#tunables_modal').modal('hide');
        });

        // Screen save!
        $('#save_button').click(function(){
          // Save properties!
          saveScreen();
        });

        // Load screen!
        loadScreen(<?php echo file_get_contents("screens/".$_GET["id"]); ?>);

      });

      // Storage for components and component types
      var instance_array = {};
      var widget_array = {
      <?php // List of widgets and their aspect ratios
        foreach(Providers::getProvidersList() as $provName => $prov) {
          $inst = new $prov["class"];
          // Aspect ratio
          $ar = $inst->shape()["width"]/$inst->shape()["height"];
          $kar = $inst->shape()["keep_aspect"] ? "true" : "false";
          // Tunable parameters!
          $tun = $inst->getTunables();
          $tunstr = array2js($tun);

          echo "'$provName': {'aspect_ratio': $ar, 'keep_aspect': $kar, 'tunables': $tunstr },\n";
        }
      ?>
      };

      function reloadImages() {
        $(".svg_widget").each(function (i, obj) {
          // Class type
          var wn = $(obj).get(0).getAttribute('data-n');
          var wi = $(obj).get(0).getAttribute('data-i');
          // Serialize tunables as well
          var tunstr = encodeURIComponent(JSON.stringify(instance_array[wi]["tunables"]));
          $(obj).attr("src", 'editor.php?action=preview&widget='+encodeURIComponent(wn)+'&tuns='+tunstr);
        });
      }

      // Generate a JSON respresentation of the screen so it can be stored in the server
      // and use as input for rendering.
      function serializeScreen() {
        var scr_desc = {
          "width":  $("#screen_width").val(),
          "height": $("#screen_height").val(),
          "widgets": [],
        };

        $(".resize-drag").each(function (i, obj) {
          var wi = $(obj).get(0).getAttribute('data-i');
          // Each widget will store its x,y components as well as w/h but they will be scaled to a 1,1 square
          // So they can be screen dimension agnostic
          var st = {};
          for (var v in instance_array[wi]["tunables"])
            st[v] = instance_array[wi]["tunables"][v]["value"];
          var comp = {
            "geo": {
              "x": $(obj).get(0).getAttribute('data-x') / $("#playground").width(),
              "y": $(obj).get(0).getAttribute('data-y') / $("#playground").height(),
              "w": $(obj).get(0).getAttribute('data-w') / $("#playground").width(),
              "h": $(obj).get(0).getAttribute('data-h') / $("#playground").height(),
            },
            "type": $(obj).get(0).getAttribute('data-n'),
            "params": st,
          };

          scr_desc["widgets"].push(comp);
        });
        return JSON.stringify(scr_desc);
      }

      // Save via AJAX
      function saveScreen() {
        var s = serializeScreen();
        $.ajax({
          type: "POST",
          url: "editor.php?action=save&id=<?php echo urlencode($_GET['id']); ?>",
          data: s,
          success: function() {
            $('#alert_saved').fadeIn('fast');
            $("#alert_saved").delay(1000).fadeOut("slow");
          }
        });
      }

      function loadScreen(ivar) {
        if (!ivar) return;

        // Load screen size
        $("#screen_width").val(ivar["width"]);
        $("#screen_height").val(ivar["height"]);
        pground_resize(false);

        // Now load widgets!
        for (var widg in ivar["widgets"]) {
          // Create widgets!
          addWidget(ivar["widgets"][widg]["type"], ivar["widgets"][widg]["params"], ivar["widgets"][widg]["geo"]);
        }
      }

      var wcounter = 0;
      function addWidget(wn, tun, geo) {
        wcounter++;

        var nsvg = jQuery('<img/>', {
          class: 'svg_widget',
        });

        var ndiv = jQuery('<div/>', {
          class: 'resize-drag',
          style: 'border:1px solid black;',
        });

        var somewidth  = $("#playground").width()/4;
        var someheight = somewidth / widget_array[wn]["aspect_ratio"];
        var x = ($("#playground").width() - somewidth)/2;
        var y = ($("#playground").height() - someheight)/2;

        // Override dimenstions
        if (geo) {
          somewidth = geo["w"] * $("#playground").width();
          someheight = geo["h"] * $("#playground").height();
          x = geo["x"] * $("#playground").width();
          y = geo["y"] * $("#playground").height();
        }

        ndiv.width (somewidth + "px");
        ndiv.height(someheight+ "px");
        nsvg.width ("100%");
        nsvg.height ("100%");

        ndiv.get(0).setAttribute('data-x', x);
        ndiv.get(0).setAttribute('data-y', y);
        ndiv.get(0).setAttribute('data-w', somewidth);
        ndiv.get(0).setAttribute('data-h', someheight);
        ndiv.get(0).setAttribute('data-n', wn);
        nsvg.get(0).setAttribute('data-n', wn);
        ndiv.get(0).setAttribute('data-i', wcounter);
        nsvg.get(0).setAttribute('data-i', wcounter);

        ndiv.get(0).style.webkitTransform =
        ndiv.get(0).style.transform =
          'translate(' + x + 'px, ' + y + 'px)';

        nsvg.appendTo(ndiv);
        ndiv.appendTo('#playground');

        instance_array[wcounter] = clone(widget_array[wn]);
        if (tun != null) {
          for (var v in tun)
            instance_array[wcounter]["tunables"][v]["value"] = tun[v];
        }

        reloadImages();
      }

    </script>

  </head>

  <body>
    <nav class="navbar navbar-fixed-top navbar-inverse">
      <div class="container">
        <div class="navbar-header">
          <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#navbar" aria-expanded="false" aria-controls="navbar">
            <span class="sr-only">Toggle navigation</span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
          </button>
          <a class="navbar-brand" href="#">E-ink cloud frame</a>
        </div>
        <div id="navbar" class="collapse navbar-collapse">
          <ul class="nav navbar-nav">
            <li><a href="admin.php">Home</a></li>
            <li class="active"><a href="editor.php">Editor</a></li>
          </ul>
        </div><!-- /.nav-collapse -->
      </div><!-- /.container -->
    </nav><!-- /.navbar -->

    <div class="container">

      <div class="row row-offcanvas row-offcanvas-right">

        <div class="col-xs-9 col-sm-9">
          <div class="jumbotron">
            <div id="playground">



            </div>
          </div>
        </div>

        <div class="col-xs-3 col-sm-3" id="sidebar">
          <div class="list-group">
            <?php
               foreach(Providers::getProvidersList() as $provName => $prov) {
            ?>
            <a href="#" class="list-group-item" onclick="addWidget('<?php echo $provName; ?>', null, null);"><?php echo $provName; ?></a>
            <?php
               }
            ?>

          </div>

          <div class="button-group">
            <button type="button" class="btn btn-primary btn-lg" id="save_button">
              Save
            </button>
          </div>

          <div id="alert_saved" class="alert alert-success initially_hidden" role="alert">
            <strong>Screen saved!</strong>
          </div>


        </div><!--/.sidebar-offcanvas-->

      </div><!--/row-->

      Width: <input type="number" min="80" max="1024" step="1" value="800" size="3" id="screen_width">
      Height: <input type="number" min="80" max="1024" step="1" value="600" size="3" id="screen_height">

      <hr>

      <footer>
        <p>&copy; 2016 David Guillen Fandos</p>
      </footer>

      <div class="modal fade" id="tunables_modal" tabindex="-1" role="dialog">
        <div class="modal-dialog">
          <div class="modal-content">
            <div class="modal-header">
              <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
              <h4 class="modal-title">Edit settings</h4>
            </div>
            <div class="modal-body" id="tunables_modal_body">
              
            </div>
            <div class="modal-footer">
              <button type="button" class="btn btn-danger" id="modal_del">Delete</button>
              <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
              <button type="button" class="btn btn-primary" id="modal_save">Save changes</button>
            </div>
          </div><!-- /.modal-content -->
        </div><!-- /.modal-dialog -->
      </div><!-- /.modal -->

    </div><!--/.container-->


    <!-- Bootstrap core JavaScript
    ================================================== -->
    <!-- Placed at the end of the document so the pages load faster -->
    <script src="js/bootstrap.min.js"></script>
    <!-- IE10 viewport hack for Surface/desktop Windows 8 bug -->
    <script src="js/ie10-viewport-bug-workaround.js"></script>
    <script src="js/offcanvas.js"></script>
  </body>
</html>

