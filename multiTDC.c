#include "multiTDC.h"

extern int TDCModules;

//This code is mainly dealing with TDC data in ancillary detector channels, not with the main focal plane TDC stuff which is dealt with in f-plane.c and was written by Retief. I (Phil) know very little about how that bit works and so will try to avoid doing anything here to break that.

multiTDC::multiTDC(int ntdc, int *TDC_channel_import, float *TDC_value_import)
{
  printf("Number of TDC channels: %d\n",128*TDCModules);
  TDChits = 0;
  int *ChannelCounter = new int[128*TDCModules];
  int *GoodChannelCounter = new int[128*TDCModules];
  
  //printf("L7\n");
  for(int i=0;i<128*TDCModules;i++)ChannelCounter[i]=0;
  for(int i=0;i<128*TDCModules;i++)GoodChannelCounter[i]=0;
  //printf("***\n");
  for(int n=0;n<ntdc;n++)
  {
    printf("n: %d \t TDC_channel_import: %d \t TDC_value_import: %d \n",n,TDC_channel_import[n],TDC_value_import[n]);

    //if(TDC_channel_import[n]==845)printf("1: TDC_channel_import[n]: %d \t ChannelCounter: %d\n",TDC_channel_import[n],ChannelCounter[TDC_channel_import[n]]);
    ChannelCounter[TDC_channel_import[n]]++;
    //if(TDC_channel_import[n]==845)printf("2: TDC_channel_import[n]: %d \t ChannelCounter: %d\n",TDC_channel_import[n],ChannelCounter[TDC_channel_import[n]]);

    if(TDC_channel_import[n]>0 && TDC_channel_import[n]<128*TDCModules && TDC_value_import[n]>PulseLimits[0] && TDC_value_import[n]<PulseLimits[1])GoodChannelCounter[TDC_channel_import[n]]+=1;
    //if(TDC_channel_import[n]==845)printf("3: TDC_channel_import[n]: %d \t ChannelCounter: %d\n",TDC_channel_import[n],ChannelCounter[TDC_channel_import[n]]);
  }

  for(int n=0;n<ntdc;n++)//Loop over and dispose of the simple events (the single hit events)
  {
    //if(TDC_channel_import[n]==845)printf("4: TDC_channel_import[n]: %d \t ChannelCounter: %d\n",TDC_channel_import[n],ChannelCounter[TDC_channel_import[n]]);
    if(TDC_channel_import[n]>=6*128+48 && TDC_channel_import[n]<128*TDCModules)
    {
      if(ChannelCounter[TDC_channel_import[n]]==0)
      {
	printf("Something has gone wrong - the number of counts for channel %d is expected to be zero but something (%d) is seen.\n",TDC_channel_import[n],ChannelCounter[TDC_channel_import[n]]);
	for(int nn=0;nn<ntdc;nn++){printf("Dump: \t ntdc: %d \t n: %d \t TDC_channel_import[n]: %d \t ChannelCounter[TDC_channel_import[n]]: %d \t TDC_value_import[n]: %f \n",
					  ntdc,nn,TDC_channel_import[nn],ChannelCounter[TDC_channel_import[nn]],TDC_value_import[nn]);}
	//Removed this condition as it clashes with one of the sort things from further down in the code. Should probably put it back at some point.
	
      }
      else if(ChannelCounter[TDC_channel_import[n]]==1)//Only store events from the last TDC -> These are the ancillary detector events
      {
	//The reason that we do this this way is to look at how many events fall outside the good beampulse - only when we have multiple hits do we need to worry about the multiple hits and this should be quicker 
	printf("\n ChannelCounter[%d]==1 \n",TDC_channel_import[n]);
	SetChannel(TDC_channel_import[n]);
	SetValue(TDC_value_import[n]);
	SetMult(ChannelCounter[TDC_channel_import[n]]);
	TDChits++;
      }
      else if(ChannelCounter[TDC_channel_import[n]]>1 && GoodChannelCounter[TDC_channel_import[n]]==1 && TDC_value_import[n]>PulseLimits[0] && TDC_value_import[n]<PulseLimits[1])
      {
	SetChannel(TDC_channel_import[n]);
	SetValue(TDC_value_import[n]);
	SetMult(ChannelCounter[TDC_channel_import[n]]);
	TDChits++;
      }
//       else if(ChannelCounter[TDC_channel_import[n]]>1 && GoodChannelCounter[TDC_channel_import[n]]==2
      else if(ChannelCounter[TDC_channel_import[n]]>1 && GoodChannelCounter[TDC_channel_import[n]]>1)
      {
	printf("The number of TDC hits within the user-defined 'good pulse' is greater than 1; the number of hits is %d. The code currently doesn't deal with this.\n",GoodChannelCounter[TDC_channel_import[n]]);
      }
      else if(ChannelCounter[TDC_channel_import[n]]>1 && GoodChannelCounter[TDC_channel_import[n]]==0)
      {
	SetChannel(TDC_channel_import[n]);
	SetValue(TDC_value_import[n]);
	TDChits++;
	SetMult(ChannelCounter[TDC_channel_import[n]]);
	//Take the first TDC event for a channel where nothing falls within the good beampulse
	ChannelCounter[TDC_channel_import[n]]*=-1;//Now set the ChannelCounter for the channel to zero as we don't want to take any more of these events - this forces the change above in the first part of the if chain. Need to fix at some point.
      }
    }
  }
//   printf("\n multiTDC.c:L74 \n");
  SetHits(TDChits);//printf("\n multiTDC.c:L75 \n");
  delete ChannelCounter;//printf("\n multiTDC.c:L76 \n");
  //delete GoodChannelCounter;printf("\n multiTDC.c:L77 \n");
}

multiTDC::~multiTDC()
{
}

void multiTDC::PrintEvent()
{
  printf("Size of event: %d\n", SizeOfEvent());
  TDCchannels.clear();
  TDCvalues.clear();
}

bool multiTDC::TestEvent()
{
  bool result = true;
  if(TDChits != TDCchannels.size()){printf("TDChits != TDCchannels.size()\n");result = false;}
  if(TDChits != TDCvalues.size()){printf("TDChits != TDCvalues.size()\n");result = false;}
  if(TDCchannels.size() != TDCvalues.size()){printf("TDCchannels.size() != TDCvalues.size()\n");result = false;}
  return result;
}

void multiTDC::ClearEvent()
{
  TDChits=0;
  TDCchannels.clear();
  TDCvalues.clear();
}

unsigned int multiTDC::SizeOfEvent()
{
  return TDChits;
}
