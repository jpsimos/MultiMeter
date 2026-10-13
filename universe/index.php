<?php
	 /*
	 	Author Jacob A. Psimos
	 */

	 require_once('include/back-end/Session.php');
	 require_once('include/front-end/TabController.php');

	 Session::CreateInstance();
	 TabController::CreateInstance();
?>
<!doctype html>
<html>
<head>
	<meta charset="utf-8">
	<meta author="jacob a psimos">
	<!--<meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate" />-->
	<!--
	<meta name="viewport" content="width=device-width, initial-scale=1.0, minimum-scale=1.0, maximum-scale=3.0">
	-->
	<title>Lab</title>

	<!--Components-->
	<script src="js/jquery-3.7.1.min.js" integrity="sha384-1H217gwSVyLSIfaLxHbE7dRb3v4mYCKbpQvzx0cegeju1MVsGrX5xXxAvs/HgeFs"></script>
	<script src="js/bootstrap.min.js" integrity="sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa"></script>
	<link rel="stylesheet" href="stylesheets/bootstrap/bootstrap.min.css" integrity="sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u" />
	<link rel="stylesheet" href="stylesheets/bootstrap/bootstrap-theme.min.css" integrity="sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp" />

	<!--Component Scripts and Stylesheets-->
	<script src="js/Helpers.js" type="text/javascript"></script>
	<script src="js/MessageBox.js" type="text/javascript"></script>
	<script src="js/Clock.js" type="text/javascript"></script>
	<script src="js/LineChart.js" type="text/javascript"></script>
	<script src="js/Tab.js" type="text/javascript"></script>
	<script src="js/TabController.js" type="text/javascript"></script>
	<link rel="stylesheet" href="stylesheets/index.css" type="text/css">
	<link rel="stylesheet" href="stylesheets/Navbar.css" type="text/css">

	<!-- Tabs Scripts and Stylesheets -->
	<?php
		TabController::GetInstance()->GetHtmlHead();
	?>
	
	<script type="text/javascript">
		$(document).ready(function(){
			<?php
				 TabController::GetInstance()->GetJsCodeForDocumentLoaded();
			?>
		});
	</script>
</head>
<body>
	<!-- Navbar -->
	<nav id="topNavBar" class="navbar navbar-inverse navbar-fixed-top bg-faded">
		<div class="container-fluid">
			<div class="navbar-header">
				<span class="navbar-brand"><span class="glyphicon glyphicon-scale"></span></span>
				<button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#topNavBarCollapse">
					<span class="icon-bar"></span>
				<span class="icon-bar"></span>
					<span class="icon-bar"></span>
				</button>
				<span class="navbar-brand no-select">Lab</span>
			</div>
			<div class="container-fluid collapse navbar-collapse" id="topNavBarCollapse">
				<ul id="topNavBarList" class="nav navbar-nav navbar-right">
				</ul>
			</div>
		</div>
	</nav>

	<!-- Tab Container -->
	<div id="tabContainerOuter" class="container-fluid">
		<?php
			 TabController::GetInstance()->GetAllTabsHtml();
		?>
	</div>

	<?php
		include('include/front-end/MessageBox.php');
	?>
</body>
</html>
