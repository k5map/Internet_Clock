/*
 Function to parse HTML reply from Weather Underground
 */

/**************************************************/
String getValuesFromKey(const String response, const String sKey)
{ 
  String sKey_ = sKey;
  
  sKey_ = "\"" + sKey + "\":";
  
  char key[sKey_.length()];
  
  sKey_.toCharArray(key, sizeof(key));
  
  int keySize = sizeof(key)-1;
    
  String strResult = "";
  
  int n = response.length();
  
  for(int i=0; i < (n-keySize-1); i++)
  {
    char c[keySize];
    
    for(int k=0; k<keySize; k++)
    {
      c[k] = response.charAt(i+k);
    }
        
    boolean isEqual = true;
    
    for(int k=0; k<keySize; k++)
    {
      if(!(c[k] == key[k]))
      {
        isEqual = false;
        break;
      }
    }
    
    if(isEqual)
    {     
      int j= i + keySize + 1;
      while(!(response.charAt(j) == ','))
      {
        strResult += response.charAt(j);        
        j++;
      }
      
      //Remove char '"'
      strResult.replace("\"","");
      break;
    }
  }
  return strResult;
}

