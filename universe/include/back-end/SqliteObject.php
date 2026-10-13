<?php
	/* Author Jacob Psimos 2017 */
	
	class SqliteObject{

		private $connection = NULL;
		private $databaseFile = '';

		public function __construct($databaseFile = '/var/db/pir2.db'){
			$this->databaseFile = $databaseFile;
			$this->connection = new SQLite3($this->databaseFile);
			
			if(!$this->connection){
				throw new Exception("Failed to open database.");
			}
			else
			{
			    $this->connection->exec("PRAGMA journal_mode = WAL");
			    $this->connection->busyTimeout(1500);
			}
		}
		
		public function __destruct()
		{
		    if($this->isOpen())
		    {
		        $this->connection->close();
		    }
		}
		
		public function isOpen()
		{
		    $isOpen = FALSE;
		    
		    if($this->connection)
		    {
		        $isOpen = TRUE;
		    }
		    
		    return $isOpen;
		}

		public function setData($query){
			if(!$this->connection){
				throw new Exception("Failed to open database.");
			}

			$wasSuccess = $this->connection->exec($query);

			if(!$wasSuccess){
				if($this->connection->lastErrorCode() == 6){
					/* Table is locked, retry a few times */
					for($retryNum = 0; $retryNum < 3; $retryNum++){
						usleep(100000);
						if($this->connection->exec($query)){
						    $wasSuccess = TRUE;
						    break;
						}
					}
				}
			}

			return $wasSuccess;
		}

		public function getData($query, $multi = FALSE){
			if(!$this->connection){
				throw new Exception("Failed to open database.");
			}

			$results = $this->connection->query($query);
			
			if(!$results){
				if($this->connection->lastErrorCode() == 6){
					for($retryNum = 0; $retryNum < 3; $retryNum++){
					    usleep(100000);
						if(($results = $this->connection->query($query))){
							break;
						}
					}
				}
			}

			if(!$results){
				throw new Exception($this->connection->lastErrorMsg());
			}

			if($multi){
				$return = array();
				while($row = $results->fetchArray(SQLITE3_ASSOC)){
					array_push($return, $row);
				}
				return $return;
			}

			$row = $results->fetchArray(SQLITE3_ASSOC);
			return $row;
		}

		public function rowExists($tableName, $whereClause){
			$tableName = $this->escapeString($tableName);

			$query = sprintf("SELECT EXISTS(SELECT * FROM `%s` WHERE %s) as `e`",
				$tableName, $whereClause);

			if($row = $this->getData($query)){
				if(isset($row['e'])){
					return $row['e'] ? TRUE : FALSE;
				}
			}
			return FALSE;
		}

		public function escapeString($str){
			if(!$this->connection){
				throw new Exception("Failed to open database.");
			}
			return $this->connection->escapeString(stripslashes($str));
		}

		public function getLastError(){
			if(!$this->connection){
				throw new Exception("Failed to open database.");
			}
			return $this->connection->lastErrorMsg();
		}

		public function insertQuery($tableName, $keyValuePairs){
			if(!$this->connection){
				throw new Exception("Failed to open database.");
			}

			$columns = '';
			$values = '';
			$numValuePairs = count($keyValuePairs);
			$index = 0;

			if($numValuePairs < 1){
				throw new Exception("Must have at least one name/value pair.");
				return NULL;
			}

			foreach($keyValuePairs as $col => $value){
				$col = $this->escapeString($col);
				$value = $this->escapeString($value);

				$colFormatStr = $index < $numValuePairs - 1 ? '`%s`,' : '`%s`';
				$valueFormatStr = $index < $numValuePairs - 1 ? "'%s'," : "'%s'";

				$columns .= sprintf($colFormatStr, $col);
				$values .= sprintf($valueFormatStr, $value);

				$index = $index + 1;
			}

			$query = sprintf("INSERT INTO `%s`(%s) VALUES(%s);", $tableName, $columns, $values);
			return $query;
		}

		public function updateQuery($tableName, $whereClause, $keyValuePairs){
			if(!$this->connection){
				throw new Exception("Failed to open database.");
			}

			$valuePairs = '';
			$len = count($keyValuePairs);
			$i = 0;

			foreach($keyValuePairs as $key => $value){
				$key = $this->escapeString($key);
				$value = $this->escapeString($value);

				$valueFormatStr = $i < $len - 1 ? "`%s`='%s'," : "`%s`='%s'";
				$valuePairs .= sprintf($valueFormatStr, $key, $value);

				$i = $i + 1;
			}

			return sprintf("UPDATE `%s` SET %s WHERE %s", $tableName, $valuePairs, $whereClause);
		}
	}
?>
