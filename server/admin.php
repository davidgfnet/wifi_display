<?php
  require_once("functions.php");

  checkUserAuth(true);

  // Render a widget
  if (isset($_GET["action"]) && $_GET["action"] == "renderthumb") {
    header('Content-type: image/png');
    $im = renderBMP($_GET["id"], 8, 600, 600);
    die($im);
  }

  if (isset($_GET["action"]) && $_GET["action"] == "delete") {
    $sid = str_replace (".", "", $_GET["id"]);
    unlink("screens/".$sid);
    die("OK");
  }

  if (isset($_GET["action"]) && $_GET["action"] == "create") {
    $sid = str_replace (".", "", $_GET["name"]);
    file_put_contents("screens/".$sid, '{"width":"800","height":"600","widgets":[]}');
    die("OK");
  }

  if (isset($_GET["action"]) && $_GET["action"] == "clone") {
    $sid = str_replace (".", "", $_GET["refname"]);
    $nid = str_replace (".", "", $_GET["newname"]);
    file_put_contents("screens/".$nid, file_get_contents("screens/".$sid));
    die("OK");
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
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>


    <!-- HTML5 shim and Respond.js for IE8 support of HTML5 elements and media queries -->
    <!--[if lt IE 9]>
      <script src="https://oss.maxcdn.com/html5shiv/3.7.2/html5shiv.min.js"></script>
      <script src="https://oss.maxcdn.com/respond/1.4.2/respond.min.js"></script>
    <![endif]-->

    <script type="text/javascript">
      function deleteScreen(sname, divid) {
        $.ajax({
          type: "GET",
          url: "admin.php?action=delete&id="+sname,
          success: function() {
            $("#"+divid).fadeOut(300, function() { $(this).remove(); });
          }
        });
      }

      function createScreen() {
        $.ajax({
          type: "GET",
          url: "admin.php?action=create&name="+encodeURI($("#newscreenname").val()),
          success: function() {
            location.reload();
          }
        });
      }

      var refclone;
      function cloneScreen(vname, vhash) {
        $('#clone_modal').modal('show');
        refclone = vname;
      }

      $(document).ready(function() {
        // Clone screen!
        $('#modal_clone_button').click(function(){
          $.ajax({
            type: "GET",
            url: "admin.php?action=clone&refname="+encodeURI(refclone)+"&newname="+encodeURI($('#clone_name').val()),
            success: function() {
              location.reload();
            }
          });
        });
      });

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
            <li class="active"><a href="admin.php">Home</a></li>
            <li><a href="editor.php">Editor</a></li>
          </ul>
        </div><!-- /.nav-collapse -->
      </div><!-- /.container -->
    </nav><!-- /.navbar -->

    <div class="container">

      <div class="row row-offcanvas row-offcanvas-right">

        <div class="col-xs-12 col-sm-12">
          <div>
            <p>Browse your existing frames and create or edit them in your browser.</p>
          </div>

          <div>
            <input type="text" id="newscreenname" placeholder="Screen name">
            <a href="#" onclick="createScreen()" class="btn btn-primary">Create</a>
          </div>

          <div id="alert_delete" class="alert alert-success initially_hidden" role="alert">
            <strong>Screen deleted!</strong>
          </div>

          <div class="row">
            <?php
               $file_list = scandir("screens");
               foreach ($file_list as $file) {
                  if ($file[0] == '.') continue;
               $fn = urlencode($file);
               $fh = md5($file);
            ?>
            <div class="col-xs-12 col-sm-4 col-md-3" id="<?php echo $fh; ?>">
              <h2><?php echo $file; ?></h2>
              <p><img class="img-responsive thumbimg" src="admin.php?action=renderthumb&id=<?php echo urlencode($file); ?>"></p>
              <a href="editor.php?id=<?php echo urlencode($file); ?>" class="btn btn-default">Edit</a>
              <a href="#" onclick="cloneScreen('<?php echo $fn; ?>', '<?php echo $fh; ?>')" class="btn btn-default">Copy</a>
              <a href="#" onclick="deleteScreen('<?php echo $fn; ?>', '<?php echo $fh; ?>')" class="btn btn-danger">Delete</a>
            </div>
            <?php
               }
            ?>

          </div><!--/row-->
        </div><!--/.col-xs-12.col-sm-9-->

      </div><!--/row-->

      <hr>

      <footer>
        <p>&copy; 2016 David Guillen Fandos</p>
      </footer>

      <div class="modal fade" id="clone_modal" tabindex="-1" role="dialog">
        <div class="modal-dialog">
          <div class="modal-content">
            <div class="modal-header">
              <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
              <h4 class="modal-title">Clone screen</h4>
            </div>

            <div class="modal-body" id="tunables_modal_body">
              <div class="form-group">
                <label for="clone_name">Screen name</label>
                <input class="form-control input-lg" type="text" id="clone_name" name="clone_name" />
              </div>
            </div>

            <div class="modal-footer">
              <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
              <button type="button" class="btn btn-primary" id="modal_clone_button">Clone</button>
            </div>
          </div><!-- /.modal-content -->
        </div><!-- /.modal-dialog -->
      </div><!-- /.modal -->

    </div><!--/.container-->


    <!-- Bootstrap core JavaScript
    ================================================== -->
    <!-- Placed at the end of the document so the pages load faster -->
    <script>window.jQuery || document.write('<script src="js/vendor/jquery.min.js"><\/script>')</script>
    <script src="js/bootstrap.min.js"></script>
    <!-- IE10 viewport hack for Surface/desktop Windows 8 bug -->
    <script src="js/ie10-viewport-bug-workaround.js"></script>
    <script src="js/offcanvas.js"></script>
  </body>
</html>

