Open Source Overview
============================
INFLO-Aimsun allows connecting the INFLO-SIM application with Aimsun for the development of AMS (Analysis Modeling and Simulation) Testbeds.

License information
-------------------
Licensed under the Aimsun End-User License Agreement. You may not use this code except in compliance with the License. You can request a copy of the License by writing to info@aimsun.com
Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

System Requirements
-------------------------
INFLO-Aimsun requires an Aimsun platformSDK license to be compiled, and an Aimsun license to be executed. The hardware requirements for the use of this application are the same as those for Aimsun alone; see https://www.aimsun.com/aimsun/tech-specs/ for details.

Aimsun runs on Windows, Linux, or Mac, but INFLO-SIM is only available for Windows; therefore, even if the interface could be compiled for other operating systems, the only practical option is Windows.

Documentation
-------------
Compile the provided code to produce a .dll. Copy the library and the provided .xml in the folder where custom plugins are stored (Aimsun 8.0\plugins).Setup the Aimsun document file	- Create the subpath along which data will be collected	- Set the direction as the External ID of the subpath	- Go to Types > GKObject > Route > Subpath.	- Add GKSubpath::INFLO::IncreasingDirection, Data Type: String	- Go to Table View and display Subpath. Set the direction of the subpath in the Increasing Direction attribute	- Create a new script, set Object Type to Subpath, and link it to the provided .py script	- Update the path of the database in line 167	- Execute the INFLO database script on the subpath; the log should display “done” if the execution is successful	- In the INFLO API dialogue, set the subpath ID, the vehicle ID’s for connected vehicles and INFLO sleep time to 10 seconds minimum. Anything less will cut off communication between INFLO, Aimsun and the database too quickly. Set the paths to syncfile (“folderlocation”/Data.txt) and database (“folderlocation”/INFLOdatabase.accbd)	- Change the Scenario Demand according to the market penetration rate required
Setup INFLO-SIM	- Go to INFLOApps > INFLOApps > bin > Release > Config > INFLOConfig.xml	- Update the path in lines 5, 18 and 19 which connect to the database, the CV Data directory and TSS Data Directory, respectively.	- Update lines 22 to 28 accordingly:		o Roadway ID = Subpath ID		o EndMM can be found in the attributes of the last section of the subpath    (NB = 21.1, SB = 21.9)		o RecurringCongestionMMLocation is recommended to be set equal to EndMM
Run a simulation	- Open INFLOApp and set Syncfile in the INFLO Settings tab	- Press Start in the INFLOApp before starting a simulation		o The INFLOApp log should display messages about speed harmonization for every 20 seconds of simulation time.  This will cause the simulation to pause for a few moments to update the database and communicate with INFLO.		o One way to check if the App is pointing to the correct folder is to look at the information in the tab INFLO Configuration
Web sites
---------
The INFLO-Aimsun software is distributed through the USDOT's JPO Open Source Application Development Portal (OSADP)
http://itsforge.net/ 
