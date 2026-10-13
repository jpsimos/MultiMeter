<?php
    // Author Jacob A. Psimos
    
    require_once(dirname(__FILE__) . '/../back-end/Session.php');
    require_once(dirname(__FILE__) . '/../back-end/SqliteObject.php');

    class TabController implements JsonSerializable
    {
        private static $sm_tabController = NULL;
        private $m_tabs = array();
        
        public static function CreateInstance()
        {
            if(!TabController::$sm_tabController)
            {
                TabController::$sm_tabController = new TabController();
            }
            
            return TabController::$sm_tabController;
        }
        
        public static function GetInstance()
        {
            if(!TabController::$sm_tabController)
            {
                throw new Exception("Must call CreateInstance first.");
            }
            
            return TabController::$sm_tabController;
        }

        private function __construct()
        {
        	$tabsPath = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..' 
        		. DIRECTORY_SEPARATOR . 'front-end/tabs';
        	
            $tabPaths = array_filter(
            	scandir($tabsPath), 
            	function($item) use ($tabsPath) {
            		return is_dir($tabsPath . DIRECTORY_SEPARATOR . $item) 
            			&& $item != '.' && $item != '..';
            	}
            );
            
            foreach($tabPaths as $tabPath)
            {
                $tabName = basename($tabPath);
                $classPath = $tabsPath . DIRECTORY_SEPARATOR . basename($tabName) 
                    . DIRECTORY_SEPARATOR . basename($tabName) . '.php';
                
                if(file_exists($classPath))
                {
                    require_once($classPath);
                    $this->m_tabs[$tabName] = new $tabName();
                }
            }
        }

        public function jsonSerialize() : array
        {
            return get_object_vars($this);
        }
        
        public function GetHtmlHead()
        {
            foreach($this->m_tabs as $tabName => $tab)
            {
                $tab->GetCss();
            }
            
            foreach($this->m_tabs as $tabName => $tab)
            {
                $tab->GetJs();
            }
        }
        
        public function GetAllTabsHtml()
        {
            foreach($this->m_tabs as $tabName => $tab)
            {
                $tab->GetHtml();
            }
        }
        
        public function GetJsCodeForDocumentLoaded()
        {
            foreach($this->m_tabs as $tabName => $tab)
            {
                printf("TabController.GetInstance().Tabs['%s'] = new %s();\r\n", $tabName, $tabName);
            }
        }
        
        public function GetTabFromJsFile($tabJsPath)
        {
            $tabPath = dirname($tabJsPath);
            $tabName = basename($tabPath);
            $tab = NULL;
            
            if(is_dir($tabPath) && isset($this->m_tabs[$tabName]))
            {
                $tab = $this->m_tabs[$tabName];    
            }
            
            if(!$tab)
            {
                throw new Exception("Tab " . $tabJsPath . ' is not valid.');
            }
            
            return $tab;
        }

    }
?>