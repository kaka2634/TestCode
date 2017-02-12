#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

#include <iostream>

using namespace ns3;

class MyObject :public Object
{
public:
   static TypeId GetTypeId(void)
   {
     static TypeId tid =TypeId("MyObject")
        .SetParent (Object::GetTypeId())
        .AddConstructor<MyObject>()
        .AddTraceSource("MyInteger",
                        "An integer value to trace",
                        "MakeTraceSourceAccessor(&MyObject::m_myInt)")
                        ;
                        return tid
   }
   MyObject(){}
   TracedValue<int32_t> m_myInt;
}

//trace sink definition
void
IntTrace (int32_t oldValue,int32_t newValue)
{
  std::out<<"Traced"<<oldValue<<"to"<<newValue<<std::endl;
}

int main (int argc, char* argv [])
{
  Ptr<MyObject> MyObject=CreateObject<MyObject>();
  myObject->TraceConnectWithoutContext("MyInteger",MakeCallback(&IntTrace));

  myObject->m_myInt=1234;
}
