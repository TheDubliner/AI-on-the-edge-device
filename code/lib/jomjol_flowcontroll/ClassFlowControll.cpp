#include "ClassFlowControll.h"

#include "Helper.h"

std::vector<HTMLInfo*> ClassFlowControll::GetAllDigital()
{
    for (int i = 0; i < FlowControll.size(); ++i)
        if (FlowControll[i]->name().compare("ClassFlowDigit") == 0)
            return ((ClassFlowDigit*) (FlowControll[i]))->GetHTMLInfo();

    std::vector<HTMLInfo*> empty;
    return empty;
}

std::vector<HTMLInfo*> ClassFlowControll::GetAllAnalog()
{
    for (int i = 0; i < FlowControll.size(); ++i)
        if (FlowControll[i]->name().compare("ClassFlowAnalog") == 0)
            return ((ClassFlowAnalog*) (FlowControll[i]))->GetHTMLInfo();

    std::vector<HTMLInfo*> empty;
    return empty;
}


void ClassFlowControll::SetInitialParameter(void)
{
    AutoStart = false;
    AutoIntervall = 10;
}

bool ClassFlowControll::isAutoStart(long &_intervall)
{
    _intervall = AutoIntervall * 60 * 1000; // AutoIntervall: Minuten -> ms
    return AutoStart;
}

ClassFlow* ClassFlowControll::CreateClassFlow(std::string _type)
{
    ClassFlow* cfc = NULL;

    _type = trim(_type);

    if (_type.compare("[MakeImage]") == 0)
        cfc = new ClassFlowMakeImage(&FlowControll);
    if (_type.compare("[Alignment]") == 0)
        cfc = new ClassFlowAlignment(&FlowControll);
    if (_type.compare("[Analog]") == 0)
        cfc = new ClassFlowAnalog(&FlowControll);
    if (_type.compare("[Digits]") == 0)
        cfc = new ClassFlowDigit(&FlowControll);
    if (_type.compare("[PostProcessing]") == 0)
    {
        cfc = new ClassFlowPostProcessing(&FlowControll); 
        flowpostprocessing = (ClassFlowPostProcessing*) cfc;
    }

    if (cfc)                            // Wird nur angehangen, falls es nicht [AutoTimer] ist, denn dieses ist für FlowControll
        FlowControll.push_back(cfc);

    if (_type.compare("[AutoTimer]") == 0)
        cfc = this;    

    return cfc;
}

void ClassFlowControll::InitFlow(std::string config)
{
    int aktFlow;
    bool handeled;
    string line;

    flowpostprocessing = NULL;

    ClassFlow* cfc;
    FILE* pFile;
    config = FormatFileName(config);
    pFile = fopen(config.c_str(), "r");

    line = "";
    handeled = true;


    char zw[1024];
    if (pFile != NULL)
    {
        fgets(zw, 1024, pFile);
        printf("%s", zw);
        line = std::string(zw);
    }

    while ((line.size() > 0) && !(feof(pFile)))
    {
        cfc = CreateClassFlow(line);
        if (cfc)
        {
            cfc->ReadParameter(pFile, line);
        }
        else
        {
            fgets(zw, 1024, pFile);
            printf("%s", zw);
            line = std::string(zw);
        }
    }

    fclose(pFile);

}

bool ClassFlowControll::doFlow(string time)
{
    bool result = true;
    for (int i = 0; i < FlowControll.size(); ++i)
        result = result && FlowControll[i]->doFlow(time);
    return result;
}

string ClassFlowControll::getReadout(bool _rawvalue = false)
{
    if (flowpostprocessing)
        return flowpostprocessing->getReadout();

    string zw = "";
    string result = "";

    for (int i = 0; i < FlowControll.size(); ++i)
    {
        zw = FlowControll[i]->getReadout();
        if (zw.length() > 0)
        {
            if (result.length() == 0)
                result = zw;
            else
                result = result + "\t" + zw;
        }
    }

    return result;
}

string ClassFlowControll::GetPrevalue()	
{
    if (flowpostprocessing)
    {
        return flowpostprocessing->GetPreValue();   
    }

    return std::string();    
}

std::string ClassFlowControll::UpdatePrevalue(std::string _newvalue)
{
    float zw;
    char* p;

    _newvalue = trim(_newvalue);
//    printf("Input UpdatePreValue: %s\n", _newvalue.c_str());

    if (_newvalue.compare("0.0") == 0)
    {
        zw = 0;
    }
    else
    {
        zw = strtof(_newvalue.c_str(), &p);
        if (zw == 0)
            return "- Error in String to Value Conversion!!! Must be of format value=123.456";
    }
    

    if (flowpostprocessing)
    {
        flowpostprocessing->SavePreValue(zw);
        return _newvalue;    
    }

    return std::string();
}

bool ClassFlowControll::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;


    if (aktparamgraph.compare("[AutoTimer]") != 0)       // Paragraph passt nich zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        if ((zerlegt[0] == "AutoStart") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
            {
                AutoStart = true;
            }
        }
        if ((zerlegt[0] == "Intervall") && (zerlegt.size() > 1))
        {
            AutoIntervall = std::stof(zerlegt[1]);
        }
    }
    return true;
}

