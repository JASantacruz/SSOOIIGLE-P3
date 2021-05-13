/*
---- Word Cleaner ----
AUTHOR: Silvestre Sanchez-Bermejo Sanchez
DATE: 16/04/2021
INCLUDES NECESARIOS: <algorithm>
                     <string>
DESCRIPCION: Limpia una string de caracteres especiales
             Para añadir otros caracteres adicionales, agregar otro case en EspecialChars
*/

bool EspecialChars(char c){
  switch(c){
  case ',':
  case '.':
  case ':':
  case ';':
  case '-':
  case '(':
  case ')':
    return true;
  default:
    return false;
  }
}


std::string cleanWord(std::string t){
  t.erase(std::remove_if(t.begin(),t.end(),&EspecialChars),t.end());
  return t;
}
