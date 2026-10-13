<?php 
    // Author Jacob A. Psimos
    
    abstract class Tab implements JsonSerializable
    {
        protected $m_tabName = '';
        protected $m_displayName = '';
        protected $m_navButtonGlyphicon = '';
        protected $m_open = FALSE;
        
        protected function __construct($tabObject)
        {
            $requiredItems = [
                'm_tabName',
                'm_displayName',
                'm_navButtonGlyphicon'
            ];
            
            foreach($requiredItems as $requiredItem)
            {
                if(!isset($tabObject[$requiredItem]))
                {
                    throw new Exception("Missing required item: " . $requiredItem);
                }
            }

            foreach($tabObject as $key => $value)
            {
                $this->$key = $value;
            }
        }
        
        public function jsonSerialize() : array
        {
        	return get_object_vars($this);
        }
        
        public function GetCss()
        {
            $cssFile = dirname(__FILE__) . '/tabs/' . $this->m_tabName . '/' . $this->m_tabName . '_Css.php';
            
            if(file_exists($cssFile))
            {
                include $cssFile;
            }
        }
        
        public function GetHtml()
        {
            $htmlFile = dirname(__FILE__) . '/tabs/' . $this->m_tabName . '/' 
            	. $this->m_tabName . '_Html.php';
            
            if(file_exists($htmlFile))
            {
                echo '<div class="container-fluid page-tab' . ($this->m_open ? '"' : ' inactive"')
                	. ' id="' . $this->m_tabName . '_tabContainer" name="' . $this->m_tabName . '">';
                include $htmlFile;
                echo '</div>';
            }
        }
        
        public function GetJs()
        {
            $jsFile = dirname(__FILE__) . '/tabs/' . $this->m_tabName . '/' . $this->m_tabName . '_Js.php';
            
            if(file_exists($jsFile))
            {
                include $jsFile;
            }
        }
        
        public function GetJsTabSuperCtorCallWithArgs()
        {
            printf("super('%s', '%s', '%s');\n"
                , $this->m_tabName, $this->m_displayName, $this->m_navButtonGlyphicon);
        }
   }
?>