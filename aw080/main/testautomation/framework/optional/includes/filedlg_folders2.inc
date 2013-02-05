'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
'*
'* short description : check the internal file dialog ( 1. part )
'*
'\******************************************************************************

testcase tFolder2


   dim FULLPATH as string
       FULLPATH = gOfficePath + "user\work\"

   Dim VerList(50) as String
   

   GetDirList ( ConvertPath ( FULLPATH ), "*" , VerList() )
   KillDirList ( VerList() )

   FileOpen

   Kontext "OeffnenDlg"
   Dateiname.SetText( ConvertPath ( FULLPATH ))
   Oeffnen.Click()

   printlog( CHR$(13) + " - Create a folder named 'aaa' twice" )
   CheckDirectoryName( "aaa" ) 

   printlog( CHR$(13) + " - Create a folder named '123' twice" )
   CheckDirectoryName( "123" )     

   printlog( CHR$(13) + " - Create a folder named 'yayayaya' twice" )
   CheckDirectoryName( "yayayaya" )

   printlog( CHR$(13) + " - Create a folder named 'ycycycyc.aaa' twice" )
   CheckDirectoryName( "ycycycyc.aaa" )
   
   Kontext "NeuerOrdner"
   if NeuerOrdner.Exists() then
      NeuerOrdner.Cancel()
   endif

   Kontext "OeffnenDlg"
   OeffnenDlg.Cancel()

endcase