// JavaScript Document

/* Data definition section */

var ps;
var FIRSTSTEP = 0;
var LASTSTEP =  8;
var alreadySent = 0;

var TOTALSTEPS = 9;

var CLEARONLOAD = 1;

var AJAXSTATE = 0;

var secConfVal = 0;
var resourceDictionary = new Object;
// label type resources
resourceDictionary.Labels = new Object();
// message type resources
resourceDictionary.Messages = new Object();
var w = window.top.document;


var stepHeaderDivArr = Array (wiz_welcome, //[0]
                wiz_title_ipv4, //[1]
                wiz_title_ipv6, //[2]
                wiz_title_time, //[3]
                wiz_title_dev_pass, //[4]
                wiz_title_net_name, //[5]
                wiz_title_security, //[6]
                wiz_title_security_confirm, //[7],
                wiz_title_finish //[8]                
                );

var stepFooterDivArr = Array (wiz_button_click_next, //[0]
                              wiz_button_click_next, //[1]
                              wiz_button_click_next, //[2]
                              wiz_button_click_next, //[3]
                              wiz_button_click_next, //[4]
                              wiz_button_click_next, //[5]
                              wiz_button_click_next, //[6]                
                              wiz_button_click_submit, //[7],
                              wiz_button_click_finish //[8]
                );


function ajaxShowWizard ()
{
  var xmlhttp;
  if (window.XMLHttpRequest)
  {
    xmlhttp = new XMLHttpRequest ();
    if (xmlhttp.overrideMimeType)
    {
      // set type accordingly to anticipated content type
      xmlhttp.overrideMimeType('text/html');
    }
  }
  else if (window.ActiveXObject)
  {
    try
    {
      xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
    }
    catch (e)
    {
      try
      {
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
      } catch (e) {}
    }
  
  }

  
  if (typeof xmlhttp != "undefined")
  {
    xmlhttp.onreadystatechange = function ()
    {
      if (xmlhttp.readyState == 4)
      {
        if (xmlhttp.status == 200 )
        {
          if (AJAXSTATE == 2)
          {
            var resp = new String (xmlhttp.responseText);
            var result = resp.match(/checkCredentials/ig);
            if (result == null)
            {
              var divTemp = w.getElementById ('wizarddiv');  
              var divChild = w.getElementById ('wizardappend');

              if (divChild)
              {
                divTemp.removeChild (divChild);
              }

              var divTemp = w.createElement ('div');
              divTemp.id = 'wizardappend';
              divTemp.innerHTML = resp;
              w.getElementById('wizarddiv').appendChild(divTemp);

              AJAXSTATE = 0;
              ajaxWizardPopup ();
            }
            //else
            //  window.location.replace ("admin.cgi?action=logon");
          }
        }
      }
    };

    AJAXSTATE = 2;
    try {
    xmlhttp.open ("GET", "/admin.cgi?action=wizard", true);
    } catch (e)
    {
      alert ("open error :"+e);
    }
    try
    {
    xmlhttp.send (null);
    } catch (e)
    {
    alert ("send error :" + e);
    }
  }
  else
    alert ("Your browser doesnot support ajax");

  return xmlhttp;
}


/* Check whether the session is valid for each step */

function ajaxIsSessnValidPerStep ()
{
  return true;
}

/* This function does a GET request for session validation. If the session is valid  */
function ajaxIsSessionValid ()
{
  var xmlhttp;
  if (window.XMLHttpRequest)
  {
    xmlhttp = new XMLHttpRequest ();
    if (xmlhttp.overrideMimeType)
    {
      // set type accordingly to anticipated content type
      xmlhttp.overrideMimeType('text/html');
    }
  }
  else if (window.ActiveXObject)
  {
    try
    {
      xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
    }
    catch (e)
    {
      try
      {
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
      } catch (e) {}
    }
  
  }

  
  if (typeof xmlhttp != "undefined")
  {
    xmlhttp.onreadystatechange = function ()
    {
      if (xmlhttp.readyState == 4)
      {
        if (xmlhttp.status == 200 )
        {
          if (AJAXSTATE == 1)
          {
            var resp = new String (xmlhttp.responseText);
            var valueIndex = resp.charAt(resp.lastIndexOf ("value=\"")+7);
            if (valueIndex == 1)
            {
              ajaxShowWizard ();
            }
            else
            {
              AJAXSTATE = 0;
              window.location.replace ("admin.cgi?action=logon");
            }
          }
        }
      }
    };

    AJAXSTATE = 1;
    try {
    xmlhttp.open ("GET", "/admin.cgi?action=sessn_validate", true);
    } catch (e)
    {
      alert ("open error ajaxIsSessionValid:"+e);
    }
    try
    {
    xmlhttp.send (null);
    } catch (e)
    {
      alert ("send error ajaxIsSessionValid:" + e);
    }
  }
  else
    alert ("Your browser doesnot support ajax");

  return xmlhttp;
}

function ajaxIsSessionValid1 ()
{
  var xmlhttp;
  if (window.XMLHttpRequest)
  {
    xmlhttp = new XMLHttpRequest ();
    if (xmlhttp.overrideMimeType)
    {
      // set type accordingly to anticipated content type
      xmlhttp.overrideMimeType('text/html');
    }
  }
  else if (window.ActiveXObject)
  {
    try
    {
      xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
    }
    catch (e)
    {
      try
      {
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
      } catch (e) {}
    }
  
  }

  
  if (typeof xmlhttp != "undefined")
  {
    xmlhttp.onreadystatechange = function ()
    {
      if (xmlhttp.readyState == 4)
      {
        if (xmlhttp.status == 200 )
        {
          var resp = new String (xmlhttp.responseText);
          var valueIndex = resp.charAt(resp.lastIndexOf ("value=\"")+7);
          if (valueIndex == 1)
          {
            //ajaxShowWizard ();
            if (w.getElementById ('wiz-progress-display'))
              w.getElementById('wiz-progress-display').style.display = "block";
            w.getElementById('fade').style.display="block";

            ajaxIsSessionValid ();
          }

          else
          {
            window.location.replace ("admin.cgi?action=logon");
          }
        }
      }
    };

    try {
    xmlhttp.open ("GET", "/admin.cgi?action=sessn_validate", true);
    } catch (e)
    {
      alert ("open error ajaxIsSessionValid1:"+e);
    }
    try
    {
    xmlhttp.send (null);
    } catch (e)
    {
      alert ("send error ajaxIsSessionValid1:" + e);
    }
  }
  else
    alert ("Your browser doesnot support ajax");

  return xmlhttp;
}

function ajaxWizardPost ()
{
	var xmlhttp;

	w.getElementById ('wizardNextBtn').disabled = true;
    w.getElementById ('wizardNextBtn').className = "input-submit-disable";
	w.getElementById ('wizardBackBtn').disabled = true;
    w.getElementById ('wizardBackBtn').className = "input-submit-disable";
	w.getElementById ('wizardCancelBtn').disabled = true;
    w.getElementById ('wizardCancelBtn').className = "input-submit-disable";
    
	if (window.XMLHttpRequest){
		xmlhttp = new XMLHttpRequest ();
		if (xmlhttp.overrideMimeType) {
			xmlhttp.overrideMimeType('text/html');
		}
	}
	else if (window.ActiveXObject){
		try {
			xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch (e){
			try{
				xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	 		} catch (e) {}
		}
	}

	if (typeof xmlhttp != "undefined"){
		try  {
			constructParamStringIP();
			var summparameters = ps;
	     
			xmlhttp.open ("POST", "/setup.cgi", false);
	    }
	    catch (e)
	    {
	      alert ("open error ajaxWizardPost:" + e);
	    }
	    try
	    {
			xmlhttp.setRequestHeader ("Content-Length", ps.length);
			xmlhttp.setRequestHeader ("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
			xmlhttp.send (summparameters);		
	    } 
	    catch (e){
	      alert ("send error ajaxWizardPost:" + e);
		}

		try  {
			constructParamStringTime();
			var summparameters = ps;
	     
			xmlhttp.open ("POST", "/setup.cgi", false);
	    }
	    catch (e)
	    {
	      alert ("open error ajaxWizardPost:" + e);
	    }
	    try
	    {
			xmlhttp.setRequestHeader ("Content-Length", ps.length);
			xmlhttp.setRequestHeader ("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
			xmlhttp.send (summparameters);		
	    } 
	    catch (e){
	      alert ("send error ajaxWizardPost:" + e);
		}		
		
		try  {
			constructParamStringPass();
			var summparameters = ps;
	     
			xmlhttp.open ("POST", "/setup.cgi", false);
	    }
	    catch (e)
	    {
	      alert ("open error ajaxWizardPost:" + e);
	    }
	    try
	    {
			xmlhttp.setRequestHeader ("Content-Length", ps.length);
			xmlhttp.setRequestHeader ("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
			xmlhttp.send (summparameters);		
	    } 
	    catch (e){
	      alert ("send error ajaxWizardPost:" + e);
		}		
		try  {
			constructParamStringSSID();
			var summparameters = ps;
	     
			xmlhttp.open ("POST", "/setup.cgi", false);
	    }
	    catch (e)
	    {
	      alert ("open error ajaxWizardPost:" + e);
	    }
	    try
	    {
			xmlhttp.setRequestHeader ("Content-Length", ps.length);
			xmlhttp.setRequestHeader ("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
			xmlhttp.send (summparameters);		
	    } 
	    catch (e){
	      alert ("send error ajaxWizardPost:" + e);
		}	
		try  {
			constructParamStringVAP1();
			var summparameters = ps;
	     
			xmlhttp.open ("POST", "/setup.cgi", false);
	    }
	    catch (e)
	    {
	      alert ("open error ajaxWizardPost:" + e);
	    }
	    try
	    {
			xmlhttp.setRequestHeader ("Content-Length", ps.length);
			xmlhttp.setRequestHeader ("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
			xmlhttp.send (summparameters);		
	    } 
	    catch (e){
	      alert ("send error ajaxWizardPost:" + e);
		}	
		

		try  {
			constructParamStringVAP2();
			var summparameters = ps;
	     
			xmlhttp.open ("POST", "/setup.cgi", false);
	    }
	    catch (e)
	    {
	      alert ("open error ajaxWizardPost:" + e);
	    }
	    try
	    {
			xmlhttp.setRequestHeader ("Content-Length", ps.length);
			xmlhttp.setRequestHeader ("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
			xmlhttp.send (summparameters);		
	    } 
	    catch (e){
	      alert ("send error ajaxWizardPost:" + e);
		}		

		try  {
			constructParamStringVAP3();
			var summparameters = ps;
	     
			xmlhttp.open ("POST", "/setup.cgi", false);
	    }
	    catch (e)
	    {
	      alert ("open error ajaxWizardPost:" + e);
	    }
	    try
	    {
			xmlhttp.setRequestHeader ("Content-Length", ps.length);
			xmlhttp.setRequestHeader ("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
			xmlhttp.send (summparameters);		
	    } 
	    catch (e){
	      alert ("send error ajaxWizardPost:" + e);
		}
		
		xmlhttp.onreadystatechange = function (){
			if (xmlhttp.readyState == 4) {
				if (xmlhttp.status == 200 ) {
					showSteps (8);
				}
			}
    	};

		try  {
			constructParamStringVAP4();
			var summparameters = ps;
	     
			xmlhttp.open ("POST", "/setup.cgi", true);
	    }
	    catch (e)
	    {
	      alert ("open error ajaxWizardPost:" + e);
	    }
	    try
	    {
			xmlhttp.setRequestHeader ("Content-Length", ps.length);
			xmlhttp.setRequestHeader ("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
			xmlhttp.send (summparameters);		
	    } 
	    catch (e){
	      alert ("send error ajaxWizardPost:" + e);
		}																
	}
	else
		alert ("Your browser doesnot support ajax");


	return xmlhttp;
}

function ajaxWizardPopup ()
{
  w.getElementById('wiz-progress-display').style.display="none";
  wizardLoad ();
  w.getElementById('light').style.display='block';
  w.getElementById('fade').style.display='block';
  w.getElementById('wizardDiv').style.display = 'block';  
  w.getElementById('wizarddiv0').style.display = 'block';  
  for (var i = 0; i < TOTALSTEPS; i++)
  {
    stepString='step'+i+'Table';
    if (i == 0)
    {
    
    //  if (validateConfigIpAddrPage () == true)
      
      w.getElementById(stepString).style.display="block";
    }
    else
      w.getElementById(stepString).style.display="none";
  }
  //ntpStep ();
}

function showWizard ()
{

  w.getElementById ('steppri0row').className = "step-tr-normal-class";
  w.getElementById ('step0row').className = "step-tr-normal-class";
  w.getElementById ('step0Label').className = "wiz-tab1";
  w.getElementById ('step0Label').style.color = "#FFFFFF";
  w.getElementById ('step0Label').style.fontWeight = "bold";
  w.getElementById ('step0Img').src = "ftv2blank.gif";

  w.getElementById ('steppri1row').className = "step-tr-normal-class";
  w.getElementById ('step1row').className = "step-tr-normal-class";
  w.getElementById ('step1Label').className = "wiz-tab1";
  w.getElementById ('step1Label').style.color = "#8E8E8E";
  w.getElementById ('step1Label').style.fontWeight = "bold";
  w.getElementById ('step1Img').src = "ftv2blank.gif";

  w.getElementById ('steppri2row').className = "step-tr-normal-class";
  w.getElementById ('step2row').className = "step-tr-normal-class";
  w.getElementById ('step2Label').className = "wiz-tab1";
  w.getElementById ('step2Label').style.color = "#8E8E8E";
  w.getElementById ('step2Label').style.fontWeight = "normal";
  w.getElementById ('step2Img').src = "ftv2blank.gif";

  w.getElementById ('steppri3row').className = "step-tr-normal-class";
  w.getElementById ('step3row').className = "step-tr-normal-class";
  w.getElementById ('step3Label').className = "wiz-tab1";
  w.getElementById ('step3Label').style.color = "#8E8E8E";
  w.getElementById ('step3Label').style.fontWeight = "normal";
  w.getElementById ('step3Img').src = "ftv2blank.gif";

  w.getElementById ('steppri4row').className = "step-tr-normal-class";
  w.getElementById ('step4row').className = "step-tr-normal-class";
  w.getElementById ('step4Label').className = "wiz-tab1";
  w.getElementById ('step4Label').style.color = "#8E8E8E";
  w.getElementById ('step4Label').style.fontWeight = "normal";
  w.getElementById ('step4Img').src = "ftv2blank.gif";

  w.getElementById ('steppri5row').className = "step-tr-normal-class";
  w.getElementById ('step5row').className = "step-tr-normal-class";
  w.getElementById ('step5Label').className = "wiz-tab1";
  w.getElementById ('step5Label').style.color = "#8E8E8E";
  w.getElementById ('step5Label').style.fontWeight = "bold";
  w.getElementById ('step5Img').src = "ftv2blank.gif";

  w.getElementById ('steppri6row').className = "step-tr-normal-class";
  w.getElementById ('step6row').className = "step-tr-normal-class";
  w.getElementById ('step6Label').className = "wiz-tab1";
  w.getElementById ('step6Label').style.color = "#8E8E8E";
  w.getElementById ('step6Label').style.fontWeight = "normal";
  w.getElementById ('step6Img').src = "ftv2blank.gif";

  w.getElementById ('steppri7row').className = "step-tr-normal-class";
  w.getElementById ('step7row').className = "step-tr-normal-class";
  w.getElementById ('step7Label').className = "wiz-tab1";
  w.getElementById ('step7Label').style.color = "#8E8E8E";
  w.getElementById ('step7Label').style.fontWeight = "normal";
  w.getElementById ('step7Img').src = "ftv2blank.gif";

  w.getElementById ('steppri8row').className = "step-tr-normal-class";
  w.getElementById ('step8row').className = "step-tr-normal-class";
  w.getElementById ('step8Label').className = "wiz-tab1";
  w.getElementById ('step8Label').style.color = "#8E8E8E";
  w.getElementById ('step8Label').style.fontWeight = "normal";
  w.getElementById ('step8Img').src = "ftv2blank.gif";

  w.getElementById ('steppri9row').className = "step-tr-normal-class";
  w.getElementById ('step9row').className = "step-tr-normal-class";
  w.getElementById ('step9Label').className = "wiz-tab1";
  w.getElementById ('step9Label').style.color = "#8E8E8E";
  w.getElementById ('step9Label').style.fontWeight = "normal";
  w.getElementById ('step9Img').src = "ftv2blank.gif";
 
  w.getElementById ('steppri10row').className = "step-tr-normal-class";
  w.getElementById ('step10row').className = "step-tr-normal-class";
  w.getElementById ('step10Label').className = "wiz-tab1";
  w.getElementById ('step10Label').style.color = "#8E8E8E";
  w.getElementById ('step10Label').style.fontWeight = "bold";  

  w.getElementById ('wizardBackBtn').disabled = false;
  w.getElementById ('wizardBackBtn').className = "input-submit-disable";

  w.getElementById ('wizardNextBtn').disabled = false;
  w.getElementById ('wizardNextBtn').className = "input-submit-hover";

  w.getElementById ('wizardCancelBtn').disabled = false;
  w.getElementById ('wizardCancelBtn').className = "input-submit";
  w.getElementById('wizarddiv0').style.display = 'block'; 
  w.getElementById('wiz-progress-display').style.display="block";
  w.getElementById('light').style.display='block';
  w.getElementById('fade').style.display='block';
  setTimeout ("ajaxWizardPopup ()", 3000);
  
  ScreenClean();
  dataToVisible(w.forms[0]);
}

function closeWizard()
{
  w.getElementById('light').style.display='none';
  w.getElementById('fade').style.display='none';
  w.getElementById('wizardDiv').style.display = 'none';  
  w.getElementById('wizarddiv0').style.display = 'none'; 
  //var hiddenPriTab = w.getElementById ('hidden-pri-tab').value;
  //var hiddenSecTab = w.getElementById ('hidden-sec-tab').value;

  //window.top.frames['treefrm'].clickPrimaryTabHandler (hiddenPriTab, hiddenSecTab);
}

function changeRow (stepId)
{
  var stepStrId = 'step'+stepId;
  var stepPriStrId = 'steppri'+stepId;
  var stepPriSepStrId = 'stepprisep'+stepId;
  var stepPriRow = stepPriStrId+'row';
  var stepPriSepRow = stepPriSepStrId+'row';
  var stepRow = stepStrId+'row';
  var stepLabel = stepStrId+'Label';
  var stepData = stepStrId+'Data';
  var stepImg = stepStrId+'Img';

  var stepStrPrevId = 'step'+(stepId-1);
  var stepPriStrPrevId = 'steppri'+(stepId-1);
  var stepPriSepStrPrevId = 'stepprisep'+(stepId-1);
  var stepRowPrev = stepStrPrevId+'row';
  var stepPriRowPrev = stepPriStrPrevId+'row';
  var stepPriSepRowPrev = stepPriSepStrPrevId+'row';
  var stepLabelPrev = stepStrPrevId+'Label';
  var stepDataPrev = stepStrPrevId+'Data';
  var stepImgPrev = stepStrPrevId+'Img';

  var stepStrNextId = 'step'+(stepId+1);
  var stepPriStrNextId = 'steppri'+(stepId+1);
  var stepPriSepStrNextId = 'stepprisep'+(stepId+1);
  var stepRowNext = stepStrNextId+'row';
  var stepPriRowNext = stepPriStrNextId+'row';
  var stepPriSepRowNext = stepPriSepStrNextId+'row';
  var stepLabelNext = stepStrNextId+'Label';
  var stepDataNext = stepStrNextId+'Data';
  var stepImgNext = stepStrNextId+'Img';

  var stepStrNextId1 = 'step'+(stepId+2);
  var stepPriStrNextId1 = 'steppri'+(stepId+2);
  var stepPriSepStrNextId1 = 'stepprisep'+(stepId+2);
  var stepRowNext1 = stepStrNextId1+'row';
  var stepPriRowNext1 = stepPriStrNextId1+'row';
  var stepPriSepRowNext1 = stepPriSepStrNextId1+'row';
  var stepLabelNext1 = stepStrNextId1+'Label';
  var stepDataNext1 = stepStrNextId1+'Data';
  var stepImgNext1 = stepStrNextId1+'Img';

  var stepStrNextId1 = 'step'+(stepId+3);
  var stepPriStrNextId1 = 'steppri'+(stepId+3);
  var stepPriSepStrNextId1 = 'stepprisep'+(stepId+3);
  var stepRowNext2 = stepStrNextId1+'row';
  var stepPriRowNext2 = stepPriStrNextId1+'row';
  var stepPriSepRowNext2 = stepPriSepStrNextId1+'row';
  var stepLabelNext2 = stepStrNextId1+'Label';
  var stepDataNext2 = stepStrNextId1+'Data';
  var stepImgNext2 = stepStrNextId1+'Img';

//alert("stepId: "+stepId);
  if (stepId == 0)
  {
    w.getElementById (stepRow).className = 'step-tr-active-class';
    w.getElementById (stepPriRow).className = 'step-tr-active-class';
    w.getElementById (stepData).className = 'wizard-nav-normal';
    w.getElementById (stepLabel).style.color = '#FFFFFF';
    w.getElementById (stepLabel).style.fontWeight = 'bold';

    w.getElementById (stepRowNext).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext).style.color = '#8E8E8E';
    w.getElementById (stepLabelNext).style.fontWeight = 'bold';

    w.getElementById (stepRowNext1).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext1).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext1).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext1).style.color = '#8E8E8E';
    w.getElementById (stepLabelNext1).style.fontWeight = 'normal';

  }

  if (stepId == 1)
  {
    w.getElementById (stepPriRowPrev).className = 'step-tr-normal-class';

    w.getElementById (stepRowPrev).className = 'step-tr-normal-class';
    w.getElementById (stepDataPrev).className = 'wizard-nav-normal';
    w.getElementById (stepLabelPrev).style.color = '#000000';
    w.getElementById (stepLabelPrev).style.fontWeight = 'bold';
  
    w.getElementById (stepRow).className = 'step-tr-normal-class';
    w.getElementById (stepPriRow).className = 'step-tr-normal-class';
    w.getElementById (stepData).className = 'wizard-nav-normal';
    w.getElementById (stepLabel).style.color = '#000000';
    w.getElementById (stepLabel).style.fontWeight = 'bold';

    w.getElementById (stepRowNext).className = 'step-tr-active-class';
    w.getElementById (stepPriRowNext).className = 'step-tr-active-class';
    w.getElementById (stepDataNext).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext).style.color = '#FFFFFF';
    w.getElementById (stepLabelNext).style.fontWeight = 'bold';
    w.getElementById (stepImgNext).src="ftv2blank.gif";

    w.getElementById (stepRowNext1).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext1).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext1).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext1).style.color = '#8E8E8E';
    w.getElementById (stepLabelNext1).style.fontWeight = 'normal';
    w.getElementById (stepImgNext1).src="ftv2blank.gif";
  }

  if (stepId == 2)
  {
    w.getElementById (stepRow).className = 'step-tr-normal-class';
    w.getElementById (stepPriRow).className = 'step-tr-normal-class';
    w.getElementById (stepData).className = 'wizard-nav-normal';
    w.getElementById (stepLabel).style.color = '#000000';
    w.getElementById (stepImg).src="wizard_check.gif";
    w.getElementById (stepLabel).style.fontWeight = 'normal';

    w.getElementById (stepRowNext).className = 'step-tr-active-class';
    w.getElementById (stepPriRowNext).className = 'step-tr-active-class';
    w.getElementById (stepDataNext).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext).style.color = '#FFFFFF';
    w.getElementById (stepLabelNext).style.fontWeight = 'bold';
    w.getElementById (stepImgNext).src="ftv2blank.gif";

    w.getElementById (stepRowNext1).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext1).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext1).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext1).style.color = '#8E8E8E';
    w.getElementById (stepLabelNext1).style.fontWeight = 'normal';
    w.getElementById (stepImgNext1).src="ftv2blank.gif";

    w.getElementById (stepRowNext2).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext2).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext2).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext2).style.color = '#8E8E8E';
    w.getElementById (stepLabelNext2).style.fontWeight = 'bold';
    w.getElementById (stepImgNext2).src="ftv2blank.gif";
  }

  if (stepId == 3)
  {
    w.getElementById (stepRow).className = 'step-tr-normal-class';
    w.getElementById (stepPriRow).className = 'step-tr-normal-class';
    w.getElementById (stepData).className = 'wizard-nav-normal';
    w.getElementById (stepLabel).style.color = '#000000';
    w.getElementById (stepImg).src = "wizard_check.gif";
    w.getElementById (stepLabel).style.fontWeight = 'normal';
  
    w.getElementById (stepRowNext).className = 'step-tr-active-class';
    w.getElementById (stepPriRowNext).className = 'step-tr-active-class';
    w.getElementById (stepDataNext).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext).style.color = '#FFFFFF';
    w.getElementById (stepLabelNext).style.fontWeight = 'normal';
    w.getElementById (stepImgNext).src = "ftv2blank.gif";

    w.getElementById (stepRowNext1).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext1).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext1).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext1).style.color = '#8E8E8E';
    w.getElementById (stepLabelNext1).style.fontWeight = 'bold';
    w.getElementById (stepImgNext1).src = "ftv2blank.gif";

    w.getElementById (stepRowNext2).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext2).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext2).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext2).style.color = '#8E8E8E';
    w.getElementById (stepLabelNext2).style.fontWeight = 'normal';

  }

  if (stepId == 4)
  {
	w.getElementById (stepRow).className = 'step-tr-normal-class';
    w.getElementById (stepPriRow).className = 'step-tr-normal-class';
    w.getElementById (stepData).className = 'wizard-nav-normal';
    w.getElementById (stepLabel).style.color = '#000000';
    w.getElementById (stepImg).src = "wizard_check.gif";
    w.getElementById (stepLabel).style.fontWeight = 'normal';
      	
    w.getElementById (stepRowNext).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext).style.color = '#000000';
    w.getElementById (stepImgNext).src = "ftv2blank.gif";
    w.getElementById (stepLabelNext).style.fontWeight = 'bold';    
  
    w.getElementById (stepRowNext1).className = 'step-tr-active-class';
    w.getElementById (stepPriRowNext1).className = 'step-tr-active-class';
    w.getElementById (stepDataNext1).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext1).style.color = '#FFFFFF';
    w.getElementById (stepLabelNext1).style.fontWeight = 'normal';
    w.getElementById (stepImgNext1).src = "ftv2blank.gif";
    
    w.getElementById (stepRowNext2).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext2).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext2).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext2).style.color = '#8E8E8E';
    w.getElementById (stepLabelNext2).style.fontWeight = 'normal';
    w.getElementById (stepImgNext2).src = "ftv2blank.gif";    
  }
  if (stepId == 5 || stepId == 6)
  {
     w.getElementById (stepRowNext).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext).style.color = '#000000';
    w.getElementById (stepImgNext).src = "wizard_check.gif";
    w.getElementById (stepLabelNext).style.fontWeight = 'normal';    
  
    w.getElementById (stepRowNext1).className = 'step-tr-active-class';
    w.getElementById (stepPriRowNext1).className = 'step-tr-active-class';
    w.getElementById (stepDataNext1).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext1).style.color = '#FFFFFF';
    w.getElementById (stepLabelNext1).style.fontWeight = 'normal';
    w.getElementById (stepImgNext1).src = "ftv2blank.gif";
    
    w.getElementById (stepRowNext2).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext2).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext2).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext2).style.color = '#8E8E8E';
    w.getElementById (stepLabelNext2).style.fontWeight = 'normal';
    w.getElementById (stepImgNext2).src = "ftv2blank.gif";      
  }  

  if (stepId == 7)
  {
     w.getElementById (stepRowNext).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext).style.color = '#000000';
    w.getElementById (stepImgNext).src = "wizard_check.gif";
    w.getElementById (stepLabelNext).style.fontWeight = 'normal';    
  
    w.getElementById (stepRowNext1).className = 'step-tr-active-class';
    w.getElementById (stepPriRowNext1).className = 'step-tr-active-class';
    w.getElementById (stepDataNext1).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext1).style.color = '#FFFFFF';
    w.getElementById (stepLabelNext1).style.fontWeight = 'normal';
    w.getElementById (stepImgNext1).src = "ftv2blank.gif";
    
    w.getElementById (stepRowNext2).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext2).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext2).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext2).style.color = '#8E8E8E';
    w.getElementById (stepLabelNext2).style.fontWeight = 'bold';
    w.getElementById (stepImgNext2).src = "ftv2blank.gif";      
  }  
    
  if (stepId == 8)
  {
    w.getElementById (stepRowNext).className = 'step-tr-normal-class';
    w.getElementById (stepPriRowNext).className = 'step-tr-normal-class';
    w.getElementById (stepDataNext).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext).style.color = '#000000';
    w.getElementById (stepImgNext).src = "wizard_check.gif";
    w.getElementById (stepLabelNext).style.fontWeight = 'normal';

    w.getElementById (stepRowNext1).className = 'step-tr-active-class';
    w.getElementById (stepPriRowNext1).className = 'step-tr-active-class';
    w.getElementById (stepDataNext1).className = 'wizard-nav-normal';
    w.getElementById (stepLabelNext1).style.color = '#FFFFFF';
    w.getElementById (stepLabelNext1).style.fontWeight = 'bold';    
    w.getElementById (stepImgNext1).src = "ftv2blank.gif";
  }
}


function showSteps (stepId)
{
  w.getElementById('stepHeaderDiv').innerHTML = stepHeaderDivArr[stepId];
  w.getElementById('stepFooterDiv').innerHTML = stepFooterDivArr[stepId];  

  changeRow (stepId);

  for (var i = 0; i < TOTALSTEPS; i++)
  {
    stepString='step'+i+'Table';
    if (stepId == i)
    {
      w.getElementById(stepString).style.display="block";

      if (i == 1)
      {
        //w.getElementById('stepxTable').style.display="block";
        w.getElementById('stepx1Table').style.display="block";
        w.getElementById('stepxx1Table').style.display="block";        
      }
      if (i == 2)
      {
        w.getElementById('stepx2Table').style.display="block";
        //w.getElementById('stepxx2Table').style.display="block";
      }      
      if (i == 3)
      {
        //w.getElementById('step3xTable').style.display="block";
      }
      if (i == 4)
      {
        //w.getElementById('step4xTable').style.display="block";
      }
      if (i == 5)
      {
        w.getElementById('step5xTable').style.display="block";
        w.getElementById('step5xxTable').style.display="block";
      }
      if (i == 6)
      {
        w.getElementById('step6xTable').style.display="block";
      }     
      if (i == 7)
      {
        w.getElementById('step7xTable').style.display="block";
      }    
      if (i == 8)
      {
        w.getElementById('step8xTable').style.display="block";
      }                   
      
    }
    else
    {
      if (i == 1)
      {
        //w.getElementById('stepxTable').style.display="none";
        w.getElementById('stepx1Table').style.display="none";
        w.getElementById('stepxx1Table').style.display="none";
      }
      if (i == 2)
      {
        w.getElementById('stepx2Table').style.display="none";
        //w.getElementById('stepxx2Table').style.display="none";
      }      
      if (i == 3)
      {
        //w.getElementById('step3xTable').style.display="none";
      }
      if (i == 4)
      {
        //w.getElementById('step4xTable').style.display="none";
      }
      if (i == 5)
      {
        w.getElementById('step5xTable').style.display="none";
        w.getElementById('step5xxTable').style.display="none";
      }
      if (i == 6)
      {
        w.getElementById('step6xTable').style.display="none";
      }
      if (i == 7)
      {
        w.getElementById('step7xTable').style.display="none";
      }          
      if (i == 8)
      {
        w.getElementById('step8xTable').style.display="none";
      }       
      w.getElementById(stepString).style.display="none";
    }
  }
	w.getElementById ('wizardBackBtn').disabled = false;
	w.getElementById ('wizardNextBtn').disabled = false;
	w.getElementById ('wizardCancelBtn').disabled = false;
   
  if (stepId == 1)
  {
    w.getElementById ('wizardBackBtn').className = 'input-submit';
    w.getElementById ('wizardBackBtn').disabled = false;
    onChangeIPv4Type();
  }
  if (stepId == 2){
		ipv6modeChange();	
		onChangeIPv6Type();
  }
  if (stepId == 3){
		ChangeTimeMode();
		ChangeNTPStatus();	 
  }
  if (stepId == 5)
	grey_ssid_hide();
  if (stepId == 6){
		setDivs();
	setKeyType1(true);
  	setKeyType2(true);
  	setKeyType3(true);
  	setKeyType4(true);
  	//setSecurityList();
  	setSecurityDivs1();
  	setSecurityDivs2();
  	setSecurityDivs3();
  	setSecurityDivs4(); 
  }
  if (stepId == 7)
  {
    //w.getElementById ('finish-ssid').innerHTML = w.getElementById ('summary-ssid').innerHTML;
    //w.getElementById ('finish-security').innerHTML = w.getElementById ('summary-security').innerHTML;          
   // w.getElementById ('wizardNextBtn').className = 'input-submit-disable';
    //w.getElementById ('wizardNextBtn').disabled = true;
    
    //setTimeout ("showSteps(8)", 2000); 
    showBackStep (stepId);
    showNextStep (stepId);
    //ajaxWizardPost ();    
  }
  else
  {
    showBackStep (stepId);
    showNextStep (stepId);
  }
}

function showBackStep (stepId)
{
  var backStepString;
  var nextBtn = w.getElementById('wizardNextBtn');  

  if (stepId == FIRSTSTEP)
  {
    backStepString ='ajaxIsSessnValidPerStep();';
    nextBtn.value = wiz_button_next;
    nextBtn.title = wiz_button_next;
  }
  else
  {
    backStepString ='ajaxIsSessnValidPerStep();';
    backStepString = backStepString + 'showSteps('+(stepId-1)+')';
  }

  var backBtn = w.getElementById('wizardBackBtn');

  if (navigator.userAgent.indexOf ('MSIE') != -1)
  {
    if (stepId == FIRSTSTEP)
      backBtn.onclick = function () {ajaxIsSessnValidPerStep ();};    
    else{
      backBtn.onclick = function () {ajaxIsSessnValidPerStep (); showSteps (stepId-1);};   
     }
     
  }
  else
  {  	
    backBtn.setAttribute('onclick', backStepString);    
  }

  if (stepId == LASTSTEP - 2)  
  {
    nextBtn.value = wiz_button_next;
    nextBtn.title = wiz_button_next;
  }
  
  if (stepId == LASTSTEP - 1)  
  {
    //ssid
    w.getElementById ('summary-ssid1').innerHTML = w.getElementById ('wl_ssid0').value;
    w.getElementById ('summary-ssid2').innerHTML = w.getElementById ('wl_ssid1').value;
    w.getElementById ('summary-ssid3').innerHTML = w.getElementById ('wl_ssid2').value;
    w.getElementById ('summary-ssid4').innerHTML = w.getElementById ('wl_ssid3').value;
    //status
    if( w.getElementById ('wl_hide_ssid0').value == "enable")
    	w.getElementById ('summary-ssid1_status').innerHTML = enable;
    else
    	w.getElementById ('summary-ssid1_status').innerHTML = disable;
    	
    if( w.getElementById ('wl_hide_ssid1').value == "enable")
    	w.getElementById ('summary-ssid2_status').innerHTML = enable;
	else
    	w.getElementById ('summary-ssid2_status').innerHTML = disable;
		    	
    if( w.getElementById ('wl_hide_ssid2').value == "enable")
    	w.getElementById ('summary-ssid3_status').innerHTML = enable;
	else
    	w.getElementById ('summary-ssid3_status').innerHTML = disable;
		    	
    if( w.getElementById ('wl_hide_ssid3').value == "enable")
    	w.getElementById ('summary-ssid4_status').innerHTML = enable;  
	else
    	w.getElementById ('summary-ssid4_status').innerHTML = disable;
		    	  	    	
    //security 
    sync_security1();
    sync_security2();
    sync_security3();
    sync_security4();
    nextBtn.value = wiz_button_submit;
    nextBtn.title = wiz_button_submit;
    if (navigator.userAgent.indexOf ('MSIE') != -1)
    {
      nextBtn.onclick = function () {ajaxIsSessnValidPerStep();showSteps(7);alert ("asdf");setTimeout ("showSteps(8)", 2000);}
    }
    else
    {
      nextBtn.setAttribute ('onclick', 'ajaxIsSessnValidPerStep();showSteps(7);alert ("test");setTimeout ("showSteps(8)", 2000);');
    }
  }
  if (stepId == LASTSTEP - 1)
  {
    //ssid
    w.getElementById ('finish-ssid1').innerHTML = w.getElementById ('summary-ssid1').innerHTML;
    w.getElementById ('finish-ssid2').innerHTML = w.getElementById ('summary-ssid2').innerHTML;
    w.getElementById ('finish-ssid3').innerHTML = w.getElementById ('summary-ssid3').innerHTML;
    w.getElementById ('finish-ssid4').innerHTML = w.getElementById ('summary-ssid4').innerHTML;
    //status
    if( w.getElementById ('wl_hide_ssid0').value == "enable")
    	w.getElementById ('finish-ssid1_status').innerHTML = enable;
    else
    	w.getElementById ('finish-ssid1_status').innerHTML = disable;
    	
    if( w.getElementById ('wl_hide_ssid1').value == "enable")
    	w.getElementById ('finish-ssid2_status').innerHTML = enable;
	  else
    	w.getElementById ('finish-ssid2_status').innerHTML = disable;
		    	
    if( w.getElementById ('wl_hide_ssid2').value == "enable")
    	w.getElementById ('finish-ssid3_status').innerHTML = enable;
	  else
    	w.getElementById ('finish-ssid3_status').innerHTML = disable;
		    	
    if( w.getElementById ('wl_hide_ssid3').value == "enable")
    	w.getElementById ('finish-ssid4_status').innerHTML = enable;  
	  else
    	w.getElementById ('finish-ssid4_status').innerHTML = disable;
    //security mode
    w.getElementById ('finish-ssid1_sec_mode').innerHTML = w.getElementById ('summary-ssid1_sec_mode').innerHTML;
    w.getElementById ('finish-ssid2_sec_mode').innerHTML = w.getElementById ('summary-ssid2_sec_mode').innerHTML;
    w.getElementById ('finish-ssid3_sec_mode').innerHTML = w.getElementById ('summary-ssid3_sec_mode').innerHTML;
    w.getElementById ('finish-ssid4_sec_mode').innerHTML = w.getElementById ('summary-ssid4_sec_mode').innerHTML;
    //security key
    w.getElementById ('finish-ssid1_sec_key').innerHTML = w.getElementById ('summary-ssid1_sec_key').innerHTML;
    w.getElementById ('finish-ssid2_sec_key').innerHTML = w.getElementById ('summary-ssid2_sec_key').innerHTML;
    w.getElementById ('finish-ssid3_sec_key').innerHTML = w.getElementById ('summary-ssid3_sec_key').innerHTML;
    w.getElementById ('finish-ssid4_sec_key').innerHTML = w.getElementById ('summary-ssid4_sec_key').innerHTML;
  }

  if (CLEARONLOAD)
  {
    if (stepId == 0);
  }
}

function constructParamStringIP ()
{
	var f = w.forms[0];

	ps="todo=wizard_save";
	ps = ps+"&this_file="+f.elements['this_file'].value;
	ps = ps+"&next_file="+f.elements['next_file'].value;
	ps = ps+"&wizard_type=ip";			
	ps = ps+"&h_ipv4_ip_type="+f.elements['h_ipv4_ip_type'].value;
	ps = ps+"&c4_wan_ip_="+f.elements['c4_wan_ip_'].value;
	ps = ps+"&c4_wan_mask_="+f.elements['c4_wan_mask_'].value;
	ps = ps+"&c4_wan_gw_="+f.elements['c4_wan_gw_'].value;
	ps = ps+"&c4_wan_dns1_="+f.elements['c4_wan_dns1_'].value;
	ps = ps+"&c4_wan_dns2_="+f.elements['c4_wan_dns2_'].value;
	ps = ps+"&h_ipv6_mode="+f.elements['h_ipv6_mode'].value;
	ps = ps+"&h_ipv6_ip_type="+f.elements['h_ipv6_ip_type'].value;
	ps = ps+"&h_ipv6_radvd6c="+f.elements['h_ipv6_radvd6c'].value;
	ps = ps+"&r_ipv6_addr="+f.elements['r_ipv6_addr'].value;
	ps = ps+"&r_ipv6_gw="+f.elements['r_ipv6_gw'].value;
	ps = ps+"&ip6_dns1="+f.elements['ip6_dns1'].value;
	ps = ps+"&ip6_dns2="+f.elements['ip6_dns2'].value;
	ps = ps+"&ip6_address="+f.elements['ip6_address'].value;
	ps = ps+"&ip6_prelen="+f.elements['ip6_prelen'].value;
	ps = ps+"&ip6_getway="+f.elements['ip6_getway'].value;
}

function constructParamStringTime ()
{
	var f = w.forms[0];

	ps="todo=wizard_save";
	ps = ps+"&this_file="+f.elements['this_file'].value;
	ps = ps+"&next_file="+f.elements['next_file'].value;	
	ps = ps+"&wizard_type=time";		
	ps = ps+"&h_tod_enable="+f.elements['h_tod_enable'].value;
	ps = ps+"&h_time_mon="+f.elements['h_time_mon'].value;
	ps = ps+"&h_time_day="+f.elements['h_time_day'].value;
	ps = ps+"&h_time_year="+f.elements['h_time_year'].value;
	ps = ps+"&h_time_zone="+escape(f.elements['h_time_zone'].value);
	ps = ps+"&h_auto_dls="+f.elements['h_auto_dls'].value;
	ps = ps+"&h_ntp_server_select="+f.elements['h_ntp_server_select'].value;
	ps = ps+"&c4_ntp_server_ip_="+f.elements['c4_ntp_server_ip_'].value;
	ps = ps+"&time_sec="+f.elements['time_sec'].value;
	ps = ps+"&time_hour="+f.elements['time_hour'].value;
	ps = ps+"&time_min="+f.elements['time_min'].value;
}

function constructParamStringPass ()
{
	var f = w.forms[0];

	ps="todo=wizard_save";
	ps = ps+"&this_file="+f.elements['this_file'].value;
	ps = ps+"&next_file="+f.elements['next_file'].value;
	ps = ps+"&wizard_type=pass";		
	ps = ps+"&h_password="+escape(f.elements['h_password'].value);	
	ps = ps+"&userName="+escape(f.elements['userName'].value);	
	ps = ps+"&sysPasswd="+escape(f.elements['sysPasswd'].value);	
	ps = ps+"&sysConfirmPasswd="+escape(f.elements['sysConfirmPasswd'].value);
}

function constructParamStringSSID ()
{
	var f = w.forms[0];

	ps="todo=wizard_save";
	ps = ps+"&this_file="+f.elements['this_file'].value;
	ps = ps+"&next_file="+f.elements['next_file'].value;
	ps = ps+"&wizard_type=ssid";		
	ps = ps+"&h_wl_hide_ssid0="+escape(f.elements['h_wl_hide_ssid0'].value);
	ps = ps+"&h_wl_hide_ssid1="+escape(f.elements['h_wl_hide_ssid1'].value);
	ps = ps+"&h_wl_hide_ssid2="+escape(f.elements['h_wl_hide_ssid2'].value);
	ps = ps+"&h_wl_hide_ssid3="+escape(f.elements['h_wl_hide_ssid3'].value);
	ps = ps+"&wl_ssid1="+escape(f.elements['wl_ssid1'].value);	
	ps = ps+"&wl_ssid2="+escape(f.elements['wl_ssid2'].value);	
	ps = ps+"&wl_ssid3="+escape(f.elements['wl_ssid3'].value);	
	ps = ps+"&wl_ssid0="+escape(f.elements['wl_ssid0'].value);	
}

function constructParamStringVAP1 ()
{
	var f = w.forms[0];

	ps="todo=wizard_save";
	ps = ps+"&this_file="+f.elements['this_file'].value;
	ps = ps+"&next_file="+f.elements['next_file'].value;	
	ps = ps+"&wizard_type=vap1";		
	ps = ps+"&h_security_mode1="+f.elements['h_security_mode1'].value;
	ps = ps+"&h_wl_auth_mode1="+f.elements['h_wl_auth_mode1'].value;
	ps = ps+"&h_wl_key_id1="+escape(f.elements['h_wl_key_id1'].value);
	ps = ps+"&h_keysize1="+f.elements['h_keysize1'].value;
	ps = ps+"&h_wpapsk_enc1="+f.elements['h_wpapsk_enc1'].value;
	ps = ps+"&c4_pradius1_ip_="+f.elements['c4_pradius1_ip_'].value;
	ps = ps+"&pradius_port1="+f.elements['pradius_port1'].value;
	ps = ps+"&pradius_psk1="+escape(f.elements['pradius_psk1'].value);		
	ps = ps+"&c4_bradius1_ip_="+f.elements['c4_bradius1_ip_'].value;
	ps = ps+"&bradius_port1="+f.elements['bradius_port1'].value;	
	ps = ps+"&bradius_psk1="+escape(f.elements['bradius_psk1'].value);
	ps = ps+"&wep_key11="+escape(f.elements['wep_key11'].value);
	ps = ps+"&wep_key21="+escape(f.elements['wep_key21'].value);
	ps = ps+"&wep_key31="+escape(f.elements['wep_key31'].value);
	ps = ps+"&wep_key41="+escape(f.elements['wep_key41'].value);	
	ps = ps+"&wpa_psk1="+escape(f.elements['wpa_psk1'].value);	
	ps = ps+"&wpapsk_lifetime1="+f.elements['wpapsk_lifetime1'].value;	
	ps = ps+"&wparadius_lifetime1="+f.elements['wparadius_lifetime1'].value;	
}

function constructParamStringVAP2 ()
{
	var f = w.forms[0];

	ps="todo=wizard_save";
	ps = ps+"&this_file="+f.elements['this_file'].value;
	ps = ps+"&next_file="+f.elements['next_file'].value;
	ps = ps+"&wizard_type=vap2";		
	ps = ps+"&h_security_mode2="+f.elements['h_security_mode2'].value;
	ps = ps+"&h_wl_auth_mode2="+f.elements['h_wl_auth_mode2'].value;
	ps = ps+"&h_wl_key_id2="+escape(f.elements['h_wl_key_id2'].value);
	ps = ps+"&h_keysize2="+f.elements['h_keysize2'].value;
	ps = ps+"&h_wpapsk_enc2="+f.elements['h_wpapsk_enc2'].value;
	ps = ps+"&c4_pradius2_ip_="+f.elements['c4_pradius2_ip_'].value;
	ps = ps+"&pradius_port2="+f.elements['pradius_port2'].value;
	ps = ps+"&pradius_psk2="+escape(f.elements['pradius_psk2'].value);		
	ps = ps+"&c4_bradius2_ip_="+f.elements['c4_bradius2_ip_'].value;
	ps = ps+"&bradius_port2="+f.elements['bradius_port2'].value;	
	ps = ps+"&bradius_psk2="+escape(f.elements['bradius_psk2'].value);
	ps = ps+"&wep_key12="+escape(f.elements['wep_key12'].value);
	ps = ps+"&wep_key22="+escape(f.elements['wep_key22'].value);
	ps = ps+"&wep_key32="+escape(f.elements['wep_key32'].value);
	ps = ps+"&wep_key42="+escape(f.elements['wep_key42'].value);
	ps = ps+"&wpa_psk2="+escape(f.elements['wpa_psk2'].value);	
	ps = ps+"&wpapsk_lifetime2="+f.elements['wpapsk_lifetime2'].value;	
	ps = ps+"&wparadius_lifetime2="+f.elements['wparadius_lifetime2'].value;	
}

function constructParamStringVAP3 ()
{
	var f = w.forms[0];

	ps="todo=wizard_save";
	ps = ps+"&this_file="+f.elements['this_file'].value;
	ps = ps+"&next_file="+f.elements['next_file'].value;	
	ps = ps+"&wizard_type=vap3";			
	ps = ps+"&h_security_mode3="+f.elements['h_security_mode3'].value;
	ps = ps+"&h_wl_auth_mode3="+f.elements['h_wl_auth_mode3'].value;
	ps = ps+"&h_wl_key_id3="+escape(f.elements['h_wl_key_id3'].value);
	ps = ps+"&h_keysize3="+f.elements['h_keysize3'].value;
	ps = ps+"&h_wpapsk_enc3="+f.elements['h_wpapsk_enc3'].value;
	ps = ps+"&c4_pradius3_ip_="+f.elements['c4_pradius3_ip_'].value;
	ps = ps+"&pradius_port3="+f.elements['pradius_port3'].value;
	ps = ps+"&pradius_psk3="+escape(f.elements['pradius_psk3'].value);	
	ps = ps+"&c4_bradius3_ip_="+f.elements['c4_bradius3_ip_'].value;
	ps = ps+"&bradius_port3="+f.elements['bradius_port3'].value;	
	ps = ps+"&bradius_psk3="+escape(f.elements['bradius_psk3'].value);
	ps = ps+"&wep_key13="+escape(f.elements['wep_key13'].value);
	ps = ps+"&wep_key23="+escape(f.elements['wep_key23'].value);
	ps = ps+"&wep_key33="+escape(f.elements['wep_key33'].value);
	ps = ps+"&wep_key43="+escape(f.elements['wep_key43'].value);
	ps = ps+"&wpa_psk3="+escape(f.elements['wpa_psk3'].value);	
	ps = ps+"&wpapsk_lifetime3="+f.elements['wpapsk_lifetime3'].value;	
	ps = ps+"&wparadius_lifetime3="+f.elements['wparadius_lifetime3'].value;	
}

function constructParamStringVAP4 ()
{
	var f = w.forms[0];

	ps="todo=wizard_save";
	ps = ps+"&this_file="+f.elements['this_file'].value;
	ps = ps+"&next_file="+f.elements['next_file'].value;	
	ps = ps+"&wizard_type=vap4";	
	ps = ps+"&h_security_mode4="+f.elements['h_security_mode4'].value;
	ps = ps+"&h_wl_auth_mode4="+f.elements['h_wl_auth_mode4'].value;
	ps = ps+"&h_wl_key_id4="+escape(f.elements['h_wl_key_id4'].value);
	ps = ps+"&h_keysize4="+f.elements['h_keysize4'].value;
	ps = ps+"&h_wpapsk_enc4="+f.elements['h_wpapsk_enc4'].value;
	ps = ps+"&c4_pradius4_ip_="+f.elements['c4_pradius4_ip_'].value;
	ps = ps+"&pradius_port4="+f.elements['pradius_port4'].value;
	ps = ps+"&pradius_psk4="+escape(f.elements['pradius_psk4'].value);
	ps = ps+"&c4_bradius4_ip_="+f.elements['c4_bradius4_ip_'].value;	
	ps = ps+"&bradius_port4="+f.elements['bradius_port4'].value;	
	ps = ps+"&bradius_psk4="+escape(f.elements['bradius_psk4'].value);	
	ps = ps+"&wep_key14="+escape(f.elements['wep_key14'].value);
	ps = ps+"&wep_key24="+escape(f.elements['wep_key24'].value);
	ps = ps+"&wep_key34="+escape(f.elements['wep_key34'].value);
	ps = ps+"&wep_key44="+escape(f.elements['wep_key44'].value);	
	ps = ps+"&wpa_psk4="+escape(f.elements['wpa_psk4'].value);	
	ps = ps+"&wpapsk_lifetime4="+f.elements['wpapsk_lifetime4'].value;	
	ps = ps+"&wparadius_lifetime4="+f.elements['wparadius_lifetime4'].value;	
}



function isStringLengthInRange(s, min, max) {
    if (s.length < min || s.length > max) {
        return false;
    } else {
        return true;
    }
}
function getStringCharsNotInRange(s, start, end) {
    var badChars = new Array();
    for (var i = 0; i < s.length; i++) {
        var code = s.charCodeAt(i);
        if (code < start || code > end) {
            var bad = "" + (i+1) + ' (' + s.charAt(i) + ')';
            badChars.push(bad);
        }
    }
    return printList(badChars);
}
function isStringCharsInRange(s, start, end) {
    for (var i = 0; i < s.length; i++) {
        var code = s.charCodeAt(i);
        if (code < start || code > end) {
            return false;
        }
    }
    return true;
}

function getInvalidWpaPskPassPhraseReason(s)
{
    var msg = "";

    var lengthBad = false;
    if (! isStringLengthInRange(s, 8, 63)) {
        msg = msg 
            + getStringLengthNotInRangeReason(s, 8, 63);
        lengthBad = true;
    }

    if (! isStringCharsInRange(s, 32, 126)) {
        if (lengthBad) {
            msg = msg + ' ';
        }
        var format = "The following characters are not allowed: %s";
        msg = msg + formatResourceString( format, new Array( getStringCharsNotInRange(s, 32, 126) ) );
    }
    return msg;
}

function getStringLengthNotInRangeReason(s, minLen, maxLen) {
    if (s.length < minLen || s.length > maxLen) {
        var format = "The string is %1 characters long, but must be between %2 and %3 characters long.";
        return formatOrderedResourceString( format, new Array( s.length, minLen, maxLen ) );
     } else {
        return "";
     }
}
function formatOrderedResourceString( format, params ) {
  var local = format;
  if( params.length && params.length > 0 ) {
    var reg = null;
    for( var i = params.length - 1; i >= 0; i-- ) {
      reg = new RegExp( "%" + (i+1), "m" );
      local = local.replace( reg, params[i] );
    }
  }
  return local;
}
function wizValidateWpaPsk(title) {
    var key = w.getElementById('wiz-wpa-personal-password').value;
    var msg = "";
    if (! isValidWpaPskPassPhrase(key)) {
       var format = "The WPA-PSK key is incorrect: %1";
       var param = new Array(); param[0] = getInvalidWpaPskPassPhraseReason(key);
       msg += formatOrderedResourceString( format, param );
    }

    if (msg.length > 0) {
        msg = "There is a problem with the WPA-PSK key:\n" + msg;
        if( title )
          msg = title + "\n" + msg;
    }

    return msg;
}

function setWizardStartup ()
{
  if (document.getElementById ('update.device.wizard.display-on-startup').checked == true)
  {
  
    document.getElementById ('update.device.wizard.display-on-startup').value = "no";
  }
  else
  {
    document.getElementById ('update.device.wizard.display-on-startup').value = "yes";
  }
}

function showNextStep (stepId)
{
  var nextStepString;
  var validateString;
  if (stepId == LASTSTEP)
    nextStepString ='';
  else
    nextStepString = 'showSteps('+(stepId+1)+');';

  validateString = 'ajaxIsSessnValidPerStep();';
  validateString = validateString + 'if(validateStep('+(stepId)+'))';
  nextStepString = validateString + nextStepString;
  var nextBtn = w.getElementById('wizardNextBtn');

  if (navigator.userAgent.indexOf ('MSIE') != -1)
  {
    if (stepId == LASTSTEP)
      nextBtn.onclick = function () { ajaxIsSessnValidPerStep(); if (validateStep(stepId)); };
    else
      nextBtn.onclick = function () { ajaxIsSessnValidPerStep(); if (validateStep(stepId)) showSteps (stepId + 1); };

  }
  else
  {
    nextBtn.setAttribute('onclick', nextStepString);
  }

//alert("stepId: "+ stepId);

  if (stepId == LASTSTEP - 1)
  {
    nextBtn.value = wiz_button_submit;
    nextBtn.title = wiz_button_submit;
    if (navigator.userAgent.indexOf ('MSIE') != -1)
  	{
  		nextBtn.onclick = function(){ajaxWizardPost()};
  	}
  	else
  	{
  		nextBtn.setAttribute ('onclick', 'ajaxWizardPost()');
  	}
  }

  if (stepId == LASTSTEP)  
  {
    nextBtn.value = wiz_finish;
    nextBtn.disabled=false;
    nextBtn.title = wiz_finish;
    if (navigator.userAgent.indexOf ('MSIE') != -1)
    {
      nextBtn.onclick = function () { ajaxIsSessnValidPerStep();closeWizard(); };
    }
    else
    {
      nextBtn.setAttribute ('onclick', 'ajaxIsSessnValidPerStep();closeWizard();');      
    }
  }

  if (stepId == LASTSTEP - 1)
  {
    w.getElementById ('wizardNextBtn').className = 'input-submit-disable';
    //w.getElementById ('wizardNextBtn').disabled = true;

    var finishString = 'showSteps('+(stepId+1)+')';
  }
  
  if (stepId == LASTSTEP)
  {
    w.getElementById ('wizardNextBtn').className = 'input-submit-hover';
    w.getElementById ('wizardNextBtn').disabled = false;

    w.getElementById ('wizardBackBtn').className = 'input-submit-disable';
    w.getElementById ('wizardBackBtn').disabled = true;
    w.getElementById ('wizardCancelBtn').className = 'input-submit-disable';
    w.getElementById ('wizardCancelBtn').disabled = true;
  }

  if (stepId == 0)
  {
    w.getElementById ('wizardBackBtn').className = 'input-submit-disable';
    w.getElementById ('wizardBackBtn').disabled = true;
  }
}

var globalField_downStroke;
function move_to_nextfield(fieldName,nextFieldName,fieldLength)
{
  //var myForm=w.forms['wiz-form'];
  var myField=w.getElementsByName(fieldName);
  myField.nextField=w.getElementsByName(nextFieldName);
  myField.maxLength=fieldLength;
  myField.onkeydown=keyDownEventHandler;
  myField.onkeyup=keyUpEventHandler;
}

function keyDownEventHandler()
{
  if(this.beforeLength == this.maxLength)
     this.beforeLength--;
  else
     this.beforeLength=this.value.length;
  globalField_downStroke=this;
}

function keyUpEventHandler()
{
  if ((this == globalField_downStroke) &&
  (this.value.length > this.beforeLength) &&
  (this.value.length >= this.maxLength)
  )
  {
    this.beforeLength = this.value.length;
    this.nextField.focus();
    this.nextField.select();
  }
  globalField_downStroke=null;
}

function wizardLoad()
{
  showSteps(0);
}

function wiztoggleElements(elements, disable) 
{
  for (var i = 0; i < elements.length; i++) 
  {
    elements[i].disabled = disable;
  }
}

function wizenableElements(elements) 
{
  wiztoggleElements(elements, false);
}

function wizdisableElements(elements) 
{
  wiztoggleElements(elements, true);
}


function wizaddQuadElements(elements, start, prefix) 
{
  for (var i = 0; i < 4; i++) 
  {
    var elementName =  prefix + i;
    elements[start+i] = 
    w.getElementsByName (elementName);
  }
}


function buttonHover (how, label, element)
{
  if (how == 'in')
  {
    element.className = 'input-submit-hover';
    var nextClassName = w.getElementById ('wizardNextBtn').className;
    if (label != 'next' && nextClassName != "input-submit-disable" && label != 'no')
    {
      w.getElementById ('wizardNextBtn').className = 'input-submit';
     // w.getElementById ('wizardNoBtn').className = 'input-submit';
    }
  }
  else
  {
    if (label != 'next' && label != 'no')
      element.className = 'input-submit';
  }
}

function xui_onkeypress(arg)
{
  CreateMeter();
  TestPassStrength();
}

function CreateMeter()
{
  AttachPasswordChangeEvent();
}

function AttachPasswordChangeEvent()
{
  var passwordInput = w.forms['wiz-form'].elements["adminpassword"][1];
  passwordInput.onkeyup = TestPassStrength;
  var confirmInput = w.forms['wiz-form'].elements["adminpasswordconfirm"][1];
  confirmInput.onkeyup = TestPassStrength;
}

function GetPasswordSettings()
{
  var passwordSettings = new Array();
  passwordSettings["CheckStrength"] = (w.forms['wiz-form'].elements["v_1_9_4"].value != undefined && w.forms['wiz-form'].elements["v_1_9_4"].value.toUpperCase() == "ENABLE");
  passwordSettings["ComplexOverride"] = (w.forms['wiz-form'].elements["v_1_13_1"][0].value != undefined && w.forms['wiz-form'].elements["v_1_13_1"][1].checked == false);
  passwordSettings["PasswordManagementMinLength"] = w.forms['wiz-form'].elements["v_1_9_1"].value;
  passwordSettings["EnableMinimumNumberOfCharClasses"] = parseInt(w.forms['wiz-form'].elements["v_1_9_2"].value) > 0;
  passwordSettings["MinimumNumberOfCharClasses"] = w.forms['wiz-form'].elements["v_1_9_2"].value;
  passwordSettings["ConsecutiveCharRepetitionLimit"] = w.forms['wiz-form'].elements["v_1_9_3"].value;
  passwordSettings["NewPasswordDiffersFromCisco"] = (w.forms['wiz-form'].elements["v_1_9_5"].value != undefined && w.forms['wiz-form'].elements["v_1_9_5"].value.toUpperCase() == "ENABLE");
  passwordSettings["NewPasswordDiffersFromUserName"] = (w.forms['wiz-form'].elements["v_1_9_6"].value != undefined && w.forms['wiz-form'].elements["v_1_9_6"].value.toUpperCase() == "ENABLE");
  return passwordSettings;
}

function GetPassStrength(passwd, userName, currPassword){
  var passwordSettings = GetPasswordSettings();
  var checkStrength = passwordSettings["CheckStrength"];
  var ComplexOverride = passwordSettings["ComplexOverride"];
  intScore = 0;

  if ((checkStrength) && (ComplexOverride)){
    if (passwd.length < passwordSettings["PasswordManagementMinLength"] || passwd.length > 64){
      return -1;
    }
    else{
       intScore = (intScore + 2 + (passwd.length - passwordSettings["PasswordManagementMinLength"]))
    }
    if (passwordSettings["EnableMinimumNumberOfCharClasses"] != undefined && passwordSettings["MinimumNumberOfCharClasses"] != undefined && 
        passwordSettings["EnableMinimumNumberOfCharClasses"] && !ValidateMinNumberOfCharClass(passwd, passwordSettings["MinimumNumberOfCharClasses"])){
      return -1;
    }
    if (passwordSettings["NewPasswordDiffersFromUserName"] != undefined && passwordSettings["NewPasswordDiffersFromUserName"] && passwd.toUpperCase().indexOf(userName.toUpperCase()) != -1){
      return -1;
    }
    if (passwordSettings["NewPasswordDiffersFromCisco"] != undefined && passwordSettings["NewPasswordDiffersFromCisco"] &&
        !(ValidatePasswordDiffersFromCisco(passwd))){
      return -1;
    }
    if (passwordSettings["ConsecutiveCharRepetitionLimit"] != undefined && passwordSettings["ConsecutiveCharRepetitionLimit"].toUpperCase() == "ENABLE" &&
        !ValidatePasswordCharRepetition(passwd, 3)){
      return -1;
    }
  }
  for (var charCounter = 0;(passwd.length !=0)&& charCounter < passwd.length - 1; charCounter++){
    var currChar = passwd.substr(charCounter, 1);
    var nextChar = passwd.substr(charCounter + 1, 1);
    if ((currChar.match(/[a-z]/) && (nextChar.match(/[a-z]/) == null)) ||
        (currChar.match(/[A-Z]/) && (nextChar.match(/[A-Z]/) == null)) ||
        (currChar.match(/\d+/) && (nextChar.match(/\d+/) == null)) ||
        (currChar.match(/[!"#$%&'()*+,-.\/:;<=>?@\[\\\]^_`{|}~]/) && (nextChar.match(/[!"#$%&'()*+,-.\/:;<=>?@\[\\\]^_`{|}~]/) == null))){
          intScore = (intScore + 1);
        }
  }

  return intScore;
}

function ValidatePasswordCharRepetition(passwd, numberOfRepetitions){
	var regEx = "/\\S*(.)\\1";
	for (repeatCount = 2; repeatCount < numberOfRepetitions + 1; repeatCount++){
		regEx = regEx + "\\1";
	}
	regEx = regEx + "\\S*/";
	if (passwd.match(eval(regEx))){
		return false;
	}
	else{
		return true;
	}
}

function ValidateMinNumberOfCharClass(passwd, minNumber){
	var isPassValid = false;
	switch (parseInt(minNumber)){
		case 1:
			isPassValid = true;
			break;
		case 2:
			if ((passwd.match(/[a-z]/) && passwd.match(/\d/)) ||
				(passwd.match(/[a-z]/) && passwd.match(/[!"#$%&'()*+,-.\/:;<=>?@\[\\\]^_`{|}~]/)) ||
				(passwd.match(/[a-z]/) && passwd.match(/[A-Z]/)) || 
				(passwd.match(/\d/) && passwd.match(/[!"#$%&'()*+,-.\/:;<=>?@\[\\\]^_`{|}~]/)) ||
				(passwd.match(/\d/) && passwd.match(/[A-Z]/)) ||
				(passwd.match(/[!"#$%&'()*+,-.\/:;<=>?@\[\\\]^_`{|}~]/) && passwd.match(/[A-Z]/))){
					isPassValid = true;
				 }
				 break;
		case 3:
			if ((passwd.match(/[a-z]/) && passwd.match(/\d/) && passwd.match(/[!"#$%&'()*+,-.\/:;<=>?@\[\\\]^_`{|}~]/)) ||
				 (passwd.match(/[a-z]/) && passwd.match(/[A-Z]/) && passwd.match(/[!"#$%&'()*+,-.\/:;<=>?@\[\\\]^_`{|}~]/)) ||
				 (passwd.match(/[a-z]/) && passwd.match(/[A-Z]/) && passwd.match(/\d/)) ||
				 (passwd.match(/[A-Z]/) && passwd.match(/\d/) && passwd.match(/[!"#$%&'()*+,-.\/:;<=>?@\[\\\]^_`{|}~]/))){
					isPassValid = true;
				 }
				 break;
		case 4:
			if (passwd.match(/[a-z]/) && passwd.match(/\d/) && passwd.match(/[!"#$%&'()*+,-.\/:;<=>?@\[\\\]^_`{|}~]/) && passwd.match(/[A-Z]/)){
				isPassValid = true;
			}
			break;
		default:
			isPassValid = false;
			break;
	}
	return isPassValid;
}

function ValidatePasswordDiffersFromCisco(passwd){
	if (passwd.toUpperCase().indexOf("CISCO") != -1 || passwd.toUpperCase().indexOf("OCSIC") != -1){
		return false;
	}
	else{
		return true;
	}
}

function TestPassStrength(){
var js_NLS_Customer_Manual_BelowMinimum = "Below Minimum" 
var js_NLS_Customer_Manual_Weak = "Weak" 
var js_NLS_Customer_Manual_Strong = "Strong"
	var srcInput = w.forms['wiz-form'].elements["adminpassword"][1];
	var userName = w.forms['wiz-form'].elements["v_1_2_1"][0].value;
        var passwordSettings = GetPasswordSettings();
        var checkStrength = passwordSettings["CheckStrength"];
        var ComplexOverride = passwordSettings["ComplexOverride"];
	var passStrength = GetPassStrength(srcInput.value, userName, null);
  var counter = 3;
	
	if ((passStrength >= 2) || ((passStrength != -1) &&(!(checkStrength && ComplexOverride)))){
    for (var imgCounter = 4; imgCounter < 14; imgCounter++){
      var imageDisplay = (passStrength / 2 >= (imgCounter - 3)) ? "" : "none";
      w.getElementById("passStrengthImg" + imgCounter).style.display = imageDisplay;
      if((passStrength / 2) >= (imgCounter - 3))
      {
        counter++;
      }
      if((counter > 3) && (counter < 9))
      {
        w.forms['wiz-form'].elements["v_1_11_2"].nextSibling.innerHTML = js_NLS_Customer_Manual_Weak;
      }
      else if(counter >= 9)
      {
        w.forms['wiz-form'].elements["v_1_11_2"].nextSibling.innerHTML = js_NLS_Customer_Manual_Strong;
      }
      else if (counter <= 3)
      {
	  w.forms['wiz-form'].elements["v_1_11_2"].nextSibling.innerHTML = js_NLS_Customer_Manual_BelowMinimum;	
      }
    }
		
		CheckConfirmPassword(w.forms['wiz-form'].elements["adminpasswordconfirm"][1], srcInput);
	}
	else{
		for (var imgCounter = 4; imgCounter < 14; imgCounter++){
			w.getElementById("passStrengthImg" + imgCounter).style.display = "none";
		}
	  w.forms['wiz-form'].elements["v_1_11_2"].nextSibling.innerHTML = js_NLS_Customer_Manual_BelowMinimum;	
		//ToggleSubmitButton(false)
	}
}

function CheckConfirmPassword(confirmField, passwordInput){
	var confirmed = confirmField.value == passwordInput.value;
	//ToggleSubmitButton(confirmed);
	return true;
}

function check_security()
{
		var key_index;
  	var m; 
  	var k;
  	var sizeOK = false;
 		var cf = document.forms[0]; 
		var msg = "";
		//ssid1	
		{ 
				var secMethod = cf.security_mode1.value;
        for(m=0; m<4; m++){
        	if(cf.wl_key_id1[m].checked){
	        	key_index = m+1;
	       	}
	    	}
				if(secMethod == "wep")
    		{    	
        	if(WEP_change1()){
     	  	for(var k=1;k<5;k++)
		  		{
						if(eval("cf.wep_key" + k + "1.value.length")>0 || k == key_index)
						{
							sizeOK =  (eval("(cf.wep_key" + k + "1.value.length == keylength)"));
							if (!sizeOK)
							{
								msg += (hexkey) ? addstr(msg_hexkey,k) : msg_asciikey;
							}
							if (hexkey)
							{
								if(eval("isHex(cf.wep_key" + k + "1.value)"))
					               ;
								else 
								{ 
									msg += addstr(msg_hexkey,k);
								}
							}
						}
	 	 			}
	 	 			if(msg.length > 1)
					{
						msg = "SSID 1:\n" +msg;
						wizard_ALERT(parent, "CON", titl_warn, "Home.htm&unit=0&vap=0", "Warn", msg, "", wizard_ABBtnOK, "");
						return false;
					}
						dataToHidden(cf);
	   		}
	   		else
	   		{
	   			if(msg.length > 1)
					{
						msg = "SSID 1:\n" +msg;
						wizard_ALERT(parent, "CON", titl_warn, "Home.htm&unit=0&vap=0", "Warn", msg, "", wizard_ABBtnOK, "");
						return false;
					}
    			dataToHidden(cf);

				}
			}
			else
			{	
        if (secMethod == "psk" || secMethod == "psk2" || secMethod == "psk_mixed")
	    	{
	      	msg+=checkWpaPass(cf.wpa_psk1.value, msg_wpa_key);
		    	msg+= checkInt(cf.wpapsk_lifetime1, msg_key_renew,600,36000,true);
	    	}
				else if(secMethod == "radius" || secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed")
        { 	
            var radius = 0;
      	    if((cf.pradius1_ip_1.value.length >0 || cf.pradius1_ip_2.value.length >0 || cf.pradius1_ip_3.value.length >0 || cf.pradius1_ip_4.value.length >0) && 
            !(cf.pradius1_ip_1.value =="0" && cf.pradius1_ip_2.value =="0" && cf.pradius1_ip_3.value =="0" && cf.pradius1_ip_4.value =="0") )
            {
                radius = 1;
                msg+= checkIp(cf.pradius1_ip_1,cf.pradius1_ip_2,cf.pradius1_ip_3,cf.pradius1_ip_4,msg_radius_server1, false);
                msg+= checkInt(cf.pradius_port1, msg_radius_port1, 1, 65534,true);
                msg+= checkBlank(cf.pradius_psk1,msg_r_login_key1); 
                msg+= checkRadiusKey(cf.pradius_psk1.value, msg_r_login_key1);
            }
            if((cf.bradius1_ip_1.value.length >0 || cf.bradius1_ip_2.value.length >0 || cf.bradius1_ip_3.value.length >0 || cf.bradius1_ip_4.value.length >0) && 
            !(cf.bradius1_ip_1.value =="0" && cf.bradius1_ip_2.value =="0" && cf.bradius1_ip_3.value =="0" && cf.bradius1_ip_4.value =="0") )
            {
                radius = 1;
                msg+= checkIp(cf.bradius1_ip_1,cf.bradius1_ip_2,cf.bradius1_ip_3,cf.bradius1_ip_4,msg_radius_server2, false);
                msg+= checkInt(cf.bradius_port1, msg_radius_port2, 1, 65534,true);
                msg+= checkBlank(cf.bradius_psk1,msg_r_login_key2);	
                msg+= checkRadiusKey(cf.bradius_psk1.value, msg_r_login_key2);
            }
            if(radius != 1)
                msg += msg_radius;
            if(secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed")
                msg+= checkInt(cf.wparadius_lifetime1, msg_key_renew,600,36000,true);
	    	}
	    	
	    	if(msg.length > 1)
				{
					msg = "SSID 1:\n" +msg;
					wizard_ALERT(parent, "CON", titl_warn, "Home.htm&unit=0&vap=0", "Warn", msg, "", wizard_ABBtnOK, "");
					return false;
				}
    		dataToHidden(cf);
     	}  
    }
		//ssid2
		{
			var secMethod = cf.security_mode2.value;
			for(m=0; m<4; m++){
        	if(cf.wl_key_id2[m].checked){
	        	key_index = m+1;
	       	}
	    	}
				if(secMethod == "wep")
    		{    	
        	if(WEP_change2()){
     	  	for(var k=1;k<5;k++)
		  		{
						if(eval("cf.wep_key" + k + "2.value.length")>0 || k == key_index)
						{
							sizeOK =  (eval("(cf.wep_key" + k + "2.value.length == keylength)"));
							if (!sizeOK)
							{
								msg += (hexkey) ? addstr(msg_hexkey,k) : msg_asciikey;
							}
							if (hexkey)
							{
								if(eval("isHex(cf.wep_key" + k + "2.value)"))
					               ;
								else 
								{ 
									msg += addstr(msg_hexkey,k);
								}
							}
						}
	 	 			}
	 	 			if(msg.length > 1)
					{
						msg = "SSID 2:\n" +msg;
						wizard_ALERT(parent, "CON", titl_warn, "Home.htm&unit=0&vap=0", "Warn", msg, "", ABBtnOK, "");
						return false;
					}
						dataToHidden(cf);
	   		}
	   		else
	   		{
	   			if(msg.length > 1)
					{
						msg = "SSID 2:\n" +msg;
						wizard_ALERT(parent, "CON", titl_warn, "Home.htm&unit=0&vap=0", "Warn", msg, "", wizard_ABBtnOK, "");
						return false;
					}
    			dataToHidden(cf);
				}
			}
			else
			{	
        if (secMethod == "psk" || secMethod == "psk2" || secMethod == "psk_mixed")
	    	{
	      	msg+=checkWpaPass(cf.wpa_psk2.value, msg_wpa_key);
		    	msg+= checkInt(cf.wpapsk_lifetime2, msg_key_renew,600,36000,true);
	    	}
				else if(secMethod == "radius" || secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed")
        { 	
            var radius = 0;
      	    if((cf.pradius2_ip_1.value.length >0 || cf.pradius2_ip_2.value.length >0 || cf.pradius2_ip_3.value.length >0 || cf.pradius2_ip_4.value.length >0) && 
            !(cf.pradius2_ip_1.value =="0" && cf.pradius2_ip_2.value =="0" && cf.pradius2_ip_3.value =="0" && cf.pradius2_ip_4.value =="0") )
            {
                radius = 1;
                msg+= checkIp(cf.pradius2_ip_1,cf.pradius2_ip_2,cf.pradius2_ip_3,cf.pradius2_ip_4,msg_radius_server1, false);
                msg+= checkInt(cf.pradius_port2, msg_radius_port1, 1, 65534,true);
                msg+= checkBlank(cf.pradius_psk2,msg_r_login_key1); 
                msg+= checkRadiusKey(cf.pradius_psk2.value, msg_r_login_key1);
            }
            if((cf.bradius2_ip_1.value.length >0 || cf.bradius2_ip_2.value.length >0 || cf.bradius2_ip_3.value.length >0 || cf.bradius2_ip_4.value.length >0) && 
            !(cf.bradius2_ip_1.value =="0" && cf.bradius2_ip_2.value =="0" && cf.bradius2_ip_3.value =="0" && cf.bradius2_ip_4.value =="0") )
            {
                radius = 1;
                msg+= checkIp(cf.bradius2_ip_1,cf.bradius2_ip_2,cf.bradius2_ip_3,cf.bradius2_ip_4,msg_radius_server2, false);
                msg+= checkInt(cf.bradius_port2, msg_radius_port2, 1, 65534,true);
                msg+= checkBlank(cf.bradius_psk2,msg_r_login_key2);	
                msg+= checkRadiusKey(cf.bradius_psk2.value, msg_r_login_key2);
            }
            if(radius != 1)
                msg += msg_radius;
            if(secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed")
                msg+= checkInt(cf.wparadius_lifetime2, msg_key_renew,600,36000,true);
	    	}
	    	
	    	if(msg.length > 1)
				{
					msg = "SSID 2:\n" +msg;
					wizard_ALERT(parent, "CON", titl_warn, "Home.htm&unit=0&vap=0", "Warn", msg, "", wizard_ABBtnOK, "");
					return false;
				}
    		dataToHidden(cf);
     	}  
		}
		//ssid3
		{
			var secMethod = cf.security_mode3.value;
			for(m=0; m<4; m++){
        	if(cf.wl_key_id3[m].checked){
	        	key_index = m+1;
	       	}
	    	}
				if(secMethod == "wep")
    		{    	
        	if(WEP_change3()){
     	  	for(var k=1;k<5;k++)
		  		{
						if(eval("cf.wep_key" + k + "3.value.length")>0 || k == key_index)
						{
							sizeOK =  (eval("(cf.wep_key" + k + "3.value.length == keylength)"));
							if (!sizeOK)
							{
								msg += (hexkey) ? addstr(msg_hexkey,k) : msg_asciikey;
							}
							if (hexkey)
							{
								if(eval("isHex(cf.wep_key" + k + "3.value)"))
					               ;
								else 
								{ 
									msg += addstr(msg_hexkey,k);
								}
							}
						}
	 	 			}
	 	 			if(msg.length > 1)
					{
						msg = "SSID 3:\n" +msg;
						wizard_ALERT(parent, "CON", titl_warn, "Home.htm&unit=0&vap=0", "Warn", msg, "", wizard_ABBtnOK, "");
						return false;
					}
						dataToHidden(cf);
	   		}
	   		else
	   		{
	   			if(msg.length > 1)
					{
						msg = "SSID 3:\n" +msg;
						wizard_ALERT(parent, "CON", titl_warn, "Home.htm&unit=0&vap=0", "Warn", msg, "", wizard_ABBtnOK, "");
						return false;
					}
    			dataToHidden(cf);
				}
			}
			else
			{	
        if (secMethod == "psk" || secMethod == "psk2" || secMethod == "psk_mixed")
	    	{
	      	msg+=checkWpaPass(cf.wpa_psk3.value, msg_wpa_key);
		    	msg+= checkInt(cf.wpapsk_lifetime3, msg_key_renew,600,36000,true);
	    	}
				else if(secMethod == "radius" || secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed")
        { 	
            var radius = 0;
      	    if((cf.pradius3_ip_1.value.length >0 || cf.pradius3_ip_2.value.length >0 || cf.pradius3_ip_3.value.length >0 || cf.pradius3_ip_4.value.length >0) && 
            !(cf.pradius3_ip_1.value =="0" && cf.pradius3_ip_2.value =="0" && cf.pradius3_ip_3.value =="0" && cf.pradius3_ip_4.value =="0") )
            {
                radius = 1;
                msg+= checkIp(cf.pradius3_ip_1,cf.pradius3_ip_2,cf.pradius3_ip_3,cf.pradius3_ip_4,msg_radius_server1, false);
                msg+= checkInt(cf.pradius_port3, msg_radius_port1, 1, 65534,true);
                msg+= checkBlank(cf.pradius_psk3,msg_r_login_key1); 
                msg+= checkRadiusKey(cf.pradius_psk3.value, msg_r_login_key1);
            }
            if((cf.bradius3_ip_1.value.length >0 || cf.bradius3_ip_2.value.length >0 || cf.bradius3_ip_3.value.length >0 || cf.bradius3_ip_4.value.length >0) && 
            !(cf.bradius3_ip_1.value =="0" && cf.bradius3_ip_2.value =="0" && cf.bradius3_ip_3.value =="0" && cf.bradius3_ip_4.value =="0") )
            {
                radius = 1;
                msg+= checkIp(cf.bradius3_ip_1,cf.bradius3_ip_2,cf.bradius3_ip_3,cf.bradius3_ip_4,msg_radius_server2, false);
                msg+= checkInt(cf.bradius_port3, msg_radius_port2, 1, 65534,true);
                msg+= checkBlank(cf.bradius_psk3,msg_r_login_key2);	
                msg+= checkRadiusKey(cf.bradius_psk3.value, msg_r_login_key2);
            }
            if(radius != 1)
                msg += msg_radius;
            if(secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed")
                msg+= checkInt(cf.wparadius_lifetime3, msg_key_renew,600,36000,true);
	    	}
	    	
	    	if(msg.length > 1)
				{
					msg = "SSID 3:\n" +msg;
					wizard_ALERT(parent, "CON", titl_warn, "Home.htm&unit=0&vap=0", "Warn", msg, "", wizard_ABBtnOK, "");
					return false;
				}
    		dataToHidden(cf);
     	}   
		}
		//ssid4
		{
			var secMethod = cf.security_mode4.value;
			for(m=0; m<4; m++){
        	if(cf.wl_key_id4[m].checked){
	        	key_index = m+1;
	       	}
	    	}
				if(secMethod == "wep")
    		{    	
        	if(WEP_change4()){
     	  	for(var k=1;k<5;k++)
		  		{
						if(eval("cf.wep_key" + k + "4.value.length")>0 || k == key_index)
						{
							sizeOK =  (eval("(cf.wep_key" + k + "4.value.length == keylength)"));
							if (!sizeOK)
							{
								msg += (hexkey) ? addstr(msg_hexkey,k) : msg_asciikey;
							}
							if (hexkey)
							{
								if(eval("isHex(cf.wep_key" + k + "4.value)"))
					               ;
								else 
								{ 
									msg += addstr(msg_hexkey,k);
								}
							}
						}
	 	 			}
	 	 			if(msg.length > 1)
					{
						msg = "SSID 4:\n" +msg;
						wizard_ALERT(parent, "CON", titl_warn, "Home.htm&unit=0&vap=0", "Warn", msg, "", wizard_ABBtnOK, "");
						return false;
					}
						dataToHidden(cf);
	   		}
	   		else
	   		{
	   			if(msg.length > 1)
					{
						msg = "SSID 4:\n" +msg;
						wizard_ALERT(parent, "CON", titl_warn, "Home.htm&unit=0&vap=0", "Warn", msg, "", wizard_ABBtnOK, "");
						return false;
					}
    			dataToHidden(cf);
				}
			}
			else
			{	
        if (secMethod == "psk" || secMethod == "psk2" || secMethod == "psk_mixed")
	    	{
	      	msg+=checkWpaPass(cf.wpa_psk4.value, msg_wpa_key);
		    	msg+= checkInt(cf.wpapsk_lifetime4, msg_key_renew,600,36000,true);
	    	}
				else if(secMethod == "radius" || secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed")
        { 	
            var radius = 0;
      	    if((cf.pradius4_ip_1.value.length >0 || cf.pradius4_ip_2.value.length >0 || cf.pradius4_ip_3.value.length >0 || cf.pradius4_ip_4.value.length >0) && 
            !(cf.pradius4_ip_1.value =="0" && cf.pradius4_ip_2.value =="0" && cf.pradius4_ip_3.value =="0" && cf.pradius4_ip_4.value =="0") )
            {
                radius = 1;
                msg+= checkIp(cf.pradius4_ip_1,cf.pradius4_ip_2,cf.pradius4_ip_3,cf.pradius4_ip_4,msg_radius_server1, false);
                msg+= checkInt(cf.pradius_port4, msg_radius_port1, 1, 65534,true);
                msg+= checkBlank(cf.pradius_psk4,msg_r_login_key1); 
                msg+= checkRadiusKey(cf.pradius_psk4.value, msg_r_login_key1);
            }
            if((cf.bradius4_ip_1.value.length >0 || cf.bradius4_ip_2.value.length >0 || cf.bradius4_ip_3.value.length >0 || cf.bradius4_ip_4.value.length >0) && 
            !(cf.bradius4_ip_1.value =="0" && cf.bradius4_ip_2.value =="0" && cf.bradius4_ip_3.value =="0" && cf.bradius4_ip_4.value =="0") )
            {
                radius = 1;
                msg+= checkIp(cf.bradius4_ip_1,cf.bradius4_ip_2,cf.bradius4_ip_3,cf.bradius4_ip_4,msg_radius_server2, false);
                msg+= checkInt(cf.bradius_port4, msg_radius_port2, 1, 65534,true);
                msg+= checkBlank(cf.bradius_psk4,msg_r_login_key2);	
                msg+= checkRadiusKey(cf.bradius_psk4.value, msg_r_login_key2);
            }
            if(radius != 1)
                msg += msg_radius;
            if(secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed")
                msg+= checkInt(cf.wparadius_lifetime4, msg_key_renew,600,36000,true);
	    	}
	    	
	    	if(msg.length > 1)
				{
					msg = "SSID 4:\n" +msg;
					wizard_ALERT(parent, "CON", titl_warn, "Home.htm", "Warn", msg, "", wizard_ABBtnOK, "");
					return false;
				}
    		dataToHidden(cf);
     	}   
		}
		return true;
} 

function validateStep (stepId)
{
  var ipaddr=0, netmask=0, gateway=0, dns1=0, dns2=0; 
	var key_index;
  var m; 
  var k;
  var sizeOK = false;
 	var cf = document.forms[0]; 
	var msg = "";
 	
		if(stepId == 0)
  	{ 
         return true;
    } 
    else if(stepId == 1)
    { 
        if(cf.ipv4_ip_type.selectedIndex == 0)
	    	{
		  		ipaddr = (cf.wan_ip_1.value << 24) |
				   				 (cf.wan_ip_2.value << 16) |
				           (cf.wan_ip_3.value << 8) |
				            cf.wan_ip_4.value;
		  	 netmask = (cf.wan_mask_1.value<< 24) |
								   (cf.wan_mask_2.value<< 16) |
									 (cf.wan_mask_3.value<< 8) |
					 			    cf.wan_mask_4.value;
		  	 gateway = (cf.wan_gw_1.value<< 24) |
									 (cf.wan_gw_2.value<< 16) |
									 (cf.wan_gw_3.value<< 8) |
					 				  cf.wan_gw_4.value;
		  	 dns1 = (cf.wan_dns1_1.value<< 24) |
				 			  (cf.wan_dns1_2.value<< 16) |
				 				(cf.wan_dns1_3.value<< 8) |
				  			 cf.wan_dns1_4.value;
	       dns2 = (cf.wan_dns2_1.value<< 24) |
				 				(cf.wan_dns2_2.value<< 16) |
				 				(cf.wan_dns2_3.value<< 8) |
				         cf.wan_dns2_4.value;

					msg+= checkIPAddress(cf.wan_ip_1,cf.wan_ip_2,cf.wan_ip_3,cf.wan_ip_4,msg_ip_address);
					msg+= checkIpMask(cf.wan_mask_1,cf.wan_mask_2,cf.wan_mask_3,cf.wan_mask_4,msg_netmask);
					msg+= checkIp(cf.wan_gw_1,cf.wan_gw_2,cf.wan_gw_3,cf.wan_gw_4,msg_gatewayip,false);
       	  msg+= checkIp(cf.wan_dns1_1,cf.wan_dns1_2,cf.wan_dns1_3,cf.wan_dns1_4,msg_DNS_ip,false);
	    	  msg+= checkIp(cf.wan_dns2_1,cf.wan_dns2_2,cf.wan_dns2_3,cf.wan_dns2_4,msg_DNS_ip,false);
		
		if(msg.length <= 0)
		{
			if (0 == (ipaddr&~netmask) || 0 == ~(ipaddr|netmask))
			{
				msg+=msg_ip_mask_mismatch;
			}
			if (0 != gateway)
			{
				if (0 == (gateway&~netmask) || 0 == ~(gateway|netmask))
				{
					msg += msg_invalid_gateway;
				}
				if ((ipaddr&netmask) != (gateway&netmask))
				{
					msg += msg_invalid_gateway_subnet;
				}
			 }	
			}
       }
       if(msg.length >0)
       {
            wizard_ALERT(parent, "CON", titl_warn, "Home.htm", "Warn", msg, "", wizard_ABBtnOK, "");
            return false;        
       }
       else 
        {
        	dataToHidden(cf);
        	return true;
        }        
    }
    else if(stepId == 2)
    {
		if(cf.ipv6_ip_type.selectedIndex == 0)
	    {		   	   
			if(cf.ipv6_mode[0].checked) {
				msg += check_v6_ip(cf.ip6_address.value,l_addr);
				if(cf.ip6_getway.value.length != 0)
					msg += check_v6_ip(cf.ip6_getway.value,l_gateway);
					msg += CheckPreLen(cf.ip6_prelen.value);
				if(cf.ip6_dns1.value.length != 0)
				    msg += check_v6_ip(cf.ip6_dns1.value,l_dns1);
				if(cf.ip6_dns2.value.length != 0)
		       	    msg += check_v6_ip(cf.ip6_dns2.value,l_dns2);
	    	}
	    }
	    if(msg.length >0) 
	    { 
	       	wizard_ALERT(parent, "CON", titl_warn, "Home.htm", "Warn", msg, "", wizard_ABBtnOK, "");
		   return false;    
	    }
	    else
	    {
        	dataToHidden(cf);
        	return true;
		}        
    } 
    else if(stepId == 3)
    { 
        if(cf.tod_enable[0].checked)
        {
    	   	 var maxday=maxday_for_month(cf.time_mon.value,cf.time_year.value);
           msg+= checkInt(cf.time_hour,msg_hour_invalid,0,23,true);
           msg+= checkInt(cf.time_min,msg_min_invalid,0,59,true);
           msg+= checkInt(cf.time_sec,msg_sec_invalid,0,59,true);
           if(parseInt(cf.time_day.value) > maxday)
            msg+=msg_date_invalid;
        }   
        else if(cf.ntp_server_select[0].checked)
        {   
            msg+= checkIp(cf.ntp_server_ip_1, cf.ntp_server_ip_2, cf.ntp_server_ip_3, cf.ntp_server_ip_4, msg_ip_address,true);
        }  
           
        if(msg.length >0)
        { 
            wizard_ALERT(parent, "CON", titl_warn, "Home.htm", "Warn", msg, "", wizard_ABBtnOK, "");
            return false;  
        }
        else
        {
        	dataToHidden(cf);
        	return true;
        } 
    }
    else if(stepId == 4)
    { 
        if(cf.sysPasswd.value != cf.sysConfirmPasswd.value) 
	       msg+= msg_pw_nomatch;
	    if(cf.sysPasswd.value != "******")
	       cf.h_password.value = 1;
        if((cf.userName.value.length < 1) || (cf.userName.value.length > 63))
           msg += msg_usname;
        if((cf.sysPasswd.value.length < 4) || (cf.sysPasswd.value.length > 63))
           msg += msg_us_password; 
         
        if(msg.length > 0)
        { 
			wizard_ALERT(parent, "CON", titl_warn, "Home.htm", "Warn", msg, "", wizard_ABBtnOK, "");
            return false;  
        }
        else
        {
        	dataToHidden(cf);
        	return true;
        }       
    }
    else if(stepId == 5)
    {  
      if(!CheckSameSSID()) 
	     return false;
	    if(!CheckSSIDBlank())
	 	   return false;
	 		else 
	 	  {	          
        	dataToHidden(cf);
        	return true;
	 		}      
    }    
		else if(stepId == 6)
    {	
    	if(check_security())
    		return true; 
  	}
  	else if(stepId == 7)
  	{
  			dataToHidden(cf);
				return true;
  	}	
    else
        return true;
}

function sync_security1()
{  
		var cf = document.forms[0]; 
   	var w = window.top.document;
		var secMethod = w.getElementById ('security_mode1').value;
		if(secMethod == "wep")
		{
			if(w.forms['wiz-form'].elements['wl_key_id1'][0].checked)
			{
				var secKey = w.getElementById ('wep_key11').value;       
			}
      else if(w.forms['wiz-form'].elements['wl_key_id1'][1].checked)
      {
        var secKey = w.getElementById ('wep_key21').value;
      }
      else if(w.forms['wiz-form'].elements['wl_key_id1'][2].checked)
      {
        var secKey = w.getElementById ('wep_key31').value;
      }
      else if(w.forms['wiz-form'].elements['wl_key_id1'][3].checked)
      {
        var secKey = w.getElementById ('wep_key41').value;
      }
    }
    else if(secMethod == "psk" || secMethod == "psk2" || secMethod == "psk_mixed")
    {
    	var secKey = w.getElementById ('wpa_psk1').value;
    }
    else if(secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed" ||secMethod == "radius")
    {
      var secKey = w.getElementById ('pradius_psk1').value;
    }
    else
    {
      var secKey = "";
    }
       
    if(secMethod == "psk")  
		w.getElementById ('summary-ssid1_sec_mode').innerHTML = ws_s_psk;
	else if(secMethod == "psk2")
		w.getElementById ('summary-ssid1_sec_mode').innerHTML = ws_s_psk2;
	else if(secMethod == "psk_mixed")
		w.getElementById ('summary-ssid1_sec_mode').innerHTML = ws_s_psk_mixed;
	else if(secMethod == "wep")
		w.getElementById ('summary-ssid1_sec_mode').innerHTML = ws_s_wep;
	else if(secMethod == "wpa_radius")
		w.getElementById ('summary-ssid1_sec_mode').innerHTML = ws_s_wpa_radius;
	else if(secMethod == "wpa2_radius")
		w.getElementById ('summary-ssid1_sec_mode').innerHTML = ws_s_wpa2_radius;
	else if(secMethod == "wpa_radius_mixed")
		w.getElementById ('summary-ssid1_sec_mode').innerHTML = ws_s_wpa_radius_mixed;
	else if(secMethod == "radius")
		w.getElementById ('summary-ssid1_sec_mode').innerHTML = ws_s_radius;
	else
		w.getElementById ('summary-ssid1_sec_mode').innerHTML =	ws_s_disabled;
    w.getElementById ('summary-ssid1_sec_key').innerHTML = secKey;         
}

function sync_security2()
{  
		var cf = document.forms[0]; 
   	var w = window.top.document;
		var secMethod = w.getElementById ('security_mode2').value;
		if(secMethod == "wep")
		{
			if(w.forms['wiz-form'].elements['wl_key_id2'][0].checked)
			{
				var secKey = w.getElementById ('wep_key12').value;       
			}
      else if(w.forms['wiz-form'].elements['wl_key_id2'][1].checked)
      {
        var secKey = w.getElementById ('wep_key22').value;
      }
      else if(w.forms['wiz-form'].elements['wl_key_id2'][2].checked)
      {
        var secKey = w.getElementById ('wep_key32').value;
      }
      else if(w.forms['wiz-form'].elements['wl_key_id2'][3].checked)
      {
        var secKey = w.getElementById ('wep_key42').value;
      }
    }
    else if(secMethod == "psk" || secMethod == "psk2" || secMethod == "psk_mixed")
    {
    	var secKey = w.getElementById ('wpa_psk2').value;
    }
    else if(secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed" ||secMethod == "radius")
    {
      var secKey = w.getElementById ('pradius_psk2').value;
    }
    else
    {
      var secKey = "";
    }
    if(secMethod == "psk")  
		w.getElementById ('summary-ssid2_sec_mode').innerHTML = ws_s_psk;
	else if(secMethod == "psk2")
		w.getElementById ('summary-ssid2_sec_mode').innerHTML = ws_s_psk2;
	else if(secMethod == "psk_mixed")
		w.getElementById ('summary-ssid2_sec_mode').innerHTML = ws_s_psk_mixed;
	else if(secMethod == "wep")
		w.getElementById ('summary-ssid2_sec_mode').innerHTML = ws_s_wep;
	else if(secMethod == "wpa_radius")
		w.getElementById ('summary-ssid2_sec_mode').innerHTML = ws_s_wpa_radius;
	else if(secMethod == "wpa2_radius")
		w.getElementById ('summary-ssid2_sec_mode').innerHTML = ws_s_wpa2_radius;
	else if(secMethod == "wpa_radius_mixed")
		w.getElementById ('summary-ssid2_sec_mode').innerHTML = ws_s_wpa_radius_mixed;
	else if(secMethod == "radius")
		w.getElementById ('summary-ssid2_sec_mode').innerHTML = ws_s_radius;
	else
		w.getElementById ('summary-ssid2_sec_mode').innerHTML =	ws_s_disabled;       
    w.getElementById ('summary-ssid2_sec_key').innerHTML = secKey;              
}

function sync_security3()
{  
		var cf = document.forms[0]; 
   	var w = window.top.document;
		var secMethod = w.getElementById ('security_mode3').value;
		if(secMethod == "wep")
		{
			if(w.forms['wiz-form'].elements['wl_key_id3'][0].checked)
			{
				var secKey = w.getElementById ('wep_key13').value;       
			}
      else if(w.forms['wiz-form'].elements['wl_key_id3'][1].checked)
      {
        var secKey = w.getElementById ('wep_key23').value;
      }
      else if(w.forms['wiz-form'].elements['wl_key_id3'][2].checked)
      {
        var secKey = w.getElementById ('wep_key33').value;
      }
      else if(w.forms['wiz-form'].elements['wl_key_id3'][3].checked)
      {
        var secKey = w.getElementById ('wep_key43').value;
      }
    }
    else if(secMethod == "psk" || secMethod == "psk2" || secMethod == "psk_mixed")
    {
    	var secKey = w.getElementById ('wpa_psk3').value;
    }
    else if(secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed" ||secMethod == "radius")
    {
      var secKey = w.getElementById ('pradius_psk3').value;
    }
    else
    {
      var secKey = "";
    }
    if(secMethod == "psk")  
		w.getElementById ('summary-ssid3_sec_mode').innerHTML = ws_s_psk;
	else if(secMethod == "psk2")
		w.getElementById ('summary-ssid3_sec_mode').innerHTML = ws_s_psk2;
	else if(secMethod == "psk_mixed")
		w.getElementById ('summary-ssid3_sec_mode').innerHTML = ws_s_psk_mixed;
	else if(secMethod == "wep")
		w.getElementById ('summary-ssid3_sec_mode').innerHTML = ws_s_wep;
	else if(secMethod == "wpa_radius")
		w.getElementById ('summary-ssid3_sec_mode').innerHTML = ws_s_wpa_radius;
	else if(secMethod == "wpa2_radius")
		w.getElementById ('summary-ssid3_sec_mode').innerHTML = ws_s_wpa2_radius;
	else if(secMethod == "wpa_radius_mixed")
		w.getElementById ('summary-ssid3_sec_mode').innerHTML = ws_s_wpa_radius_mixed;
	else if(secMethod == "radius")
		w.getElementById ('summary-ssid3_sec_mode').innerHTML = ws_s_radius;
	else
		w.getElementById ('summary-ssid3_sec_mode').innerHTML =	ws_s_disabled;
       
    w.getElementById ('summary-ssid3_sec_key').innerHTML = secKey;         
}

function sync_security4()
{  
		var cf = document.forms[0]; 
   	var w = window.top.document;
		var secMethod = w.getElementById ('security_mode4').value;
		if(secMethod == "wep")
		{
			if(w.forms['wiz-form'].elements['wl_key_id4'][0].checked)
			{
				var secKey = w.getElementById ('wep_key14').value;       
			}
      else if(w.forms['wiz-form'].elements['wl_key_id4'][1].checked)
      {
        var secKey = w.getElementById ('wep_key24').value;
      }
      else if(w.forms['wiz-form'].elements['wl_key_id4'][2].checked)
      {
        var secKey = w.getElementById ('wep_key34').value;
      }
      else if(w.forms['wiz-form'].elements['wl_key_id4'][3].checked)
      {
        var secKey = w.getElementById ('wep_key44').value;
      }
    }
    else if(secMethod == "psk" || secMethod == "psk2" || secMethod == "psk_mixed")
    {
    	var secKey = w.getElementById ('wpa_psk4').value;
    }
    else if(secMethod == "wpa_radius" || secMethod == "wpa2_radius" || secMethod == "wpa_radius_mixed" ||secMethod == "radius")
    {
      var secKey = w.getElementById ('pradius_psk4').value;
    }
    else
    {
      var secKey = "";
    }
    if(secMethod == "psk")  
		w.getElementById ('summary-ssid4_sec_mode').innerHTML = ws_s_psk;
	else if(secMethod == "psk2")
		w.getElementById ('summary-ssid4_sec_mode').innerHTML = ws_s_psk2;
	else if(secMethod == "psk_mixed")
		w.getElementById ('summary-ssid4_sec_mode').innerHTML = ws_s_psk_mixed;
	else if(secMethod == "wep")
		w.getElementById ('summary-ssid4_sec_mode').innerHTML = ws_s_wep;
	else if(secMethod == "wpa_radius")
		w.getElementById ('summary-ssid4_sec_mode').innerHTML = ws_s_wpa_radius;
	else if(secMethod == "wpa2_radius")
		w.getElementById ('summary-ssid4_sec_mode').innerHTML = ws_s_wpa2_radius;
	else if(secMethod == "wpa_radius_mixed")
		w.getElementById ('summary-ssid4_sec_mode').innerHTML = ws_s_wpa_radius_mixed;
	else if(secMethod == "radius")
		w.getElementById ('summary-ssid4_sec_mode').innerHTML = ws_s_radius;
	else
		w.getElementById ('summary-ssid4_sec_mode').innerHTML =	ws_s_disabled;
    w.getElementById ('summary-ssid4_sec_key').innerHTML = secKey;         
}

function clearKeys1()
{
    var cf = document.forms[0];
    cf.wep_key11.value = cf.wep_key21.value = cf.wep_key31.value = cf.wep_key41.value = "";
}

function clearKeys2()
{
    var cf = document.forms[0];
    cf.wep_key12.value = cf.wep_key22.value = cf.wep_key32.value = cf.wep_key42.value = "";
}
function clearKeys3()
{
    var cf = document.forms[0];
    cf.wep_key13.value = cf.wep_key23.value = cf.wep_key33.value = cf.wep_key43.value = "";
}

function clearKeys4()
{
    var cf = document.forms[0];
    cf.wep_key14.value = cf.wep_key24.value = cf.wep_key34.value = cf.wep_key44.value = "";
} 
