#include "HelloWorldPubSubTypes.h"
#include "PeoplePubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

using namespace eprosima::fastdds::dds;

class HelloWorldSubscriber {

private:
	DomainParticipant* participant_;

	Subscriber* subscriber_;

	DataReader* reader1_;
	DataReader* reader2_;//就该是两个

	Topic* topic1_;      //就该是两个
	Topic* topic2_;

	TypeSupport type1_;
	TypeSupport type2_;
	class SubListener : public DataReaderListener
	{
	public:

		SubListener()
			: samples_(0)
		{
		}

		~SubListener() override
		{
		}
		//内置写法，不用改
		void on_subscription_matched(
			DataReader*,
			const SubscriptionMatchedStatus& info) override
		{
			
			if (info.current_count_change == 1)
			{
				
				std::cout << "Subscriber matched." << std::endl;
			}
			else if (info.current_count_change == -1)
			{
			
				std::cout << "Subscriber unmatched." << std::endl;
			}
			else
			{	
				
				std::cout << info.current_count_change
					<< " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
			}
		}
		
		//error, I dont know how to parse different topics........
		void on_data_available(
			DataReader* reader) override
		{
			
			SampleInfo info1;
			
			if (
				reader->take_next_sample(&hello_, &info1) == ReturnCode_t::RETCODE_OK
				)
			{
				//std::cout << "hello_info1.valid_data" << info1.valid_data << std::endl;
				if (
					info1.valid_data
					)
				{	
					

					
					std::cout << "Message_HelloWorld: " << hello_.message() << " with hello  index: " << hello_.index()
						<< " RECEIVED." << std::endl;
					
				}
			}

			/*if (
				reader->take_next_sample(&people_, &info1) == ReturnCode_t::RETCODE_OK
				)
			{
				std::cout << "people_info1.valid_data" << info1.valid_data << std::endl;
				if (
					info1.valid_data
					)
				{
				std::cout << "Message_People    : " << people_.message() << " with people index: " << people_.index()
						<< " RECEIVED." << std::endl;
				}
			}*/
			samples_++;

			
		}




		HelloWorld hello_;
		People people_;
		std::atomic_int samples_;


	} listener_;


public:

	HelloWorldSubscriber():
		participant_(nullptr)
		, subscriber_(nullptr)
		, topic1_(nullptr)
		, topic2_(nullptr)
		, reader1_(nullptr)
		, reader2_(nullptr)
		, type1_(new HelloWorldPubSubType())
		, type2_(new PeoplePubSubType())
	{
	}

	virtual ~HelloWorldSubscriber()
	{
		if (reader1_ != nullptr 
			|| 
			reader2_ != nullptr
			)
		{	
			//std::cout << "析构reader1=" << reader1_ << " "
				//<< "析构reader2=" << reader2_ << std::endl;
			subscriber_->delete_datareader(reader1_);
			//subscriber_->delete_datareader(reader2_);
		}
		if (topic1_ != nullptr 
			||
			topic2_ != nullptr
			)
		{
			
			participant_->delete_topic(topic1_);
			participant_->delete_topic(topic2_);
		}
		if (subscriber_ != nullptr)
		{
			participant_->delete_subscriber(subscriber_);
		}
		DomainParticipantFactory::get_instance()->delete_participant(participant_);
	}

	//!Initialize the subscriber
	bool init()
	{
		DomainParticipantQos participantQos;
		participantQos.name("Participant_subscriber");
		participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

		if (participant_ == nullptr)
		{
			return false;
		}

		
		type1_.register_type(participant_);
		type2_.register_type(participant_);

		

		// Create the subscriptions Topic
		topic1_ = participant_->create_topic("HelloWorldTopic", "HelloWorld", TOPIC_QOS_DEFAULT);
		topic2_ = participant_->create_topic("PeopleTopic", "People", TOPIC_QOS_DEFAULT);

		
	
		if (topic1_ == nullptr
			||
			topic2_ == nullptr
			)
		{
			
			return false;
		}

		// Create the Subscriber
		subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

		if (subscriber_ == nullptr)
		{
			return false;
		}

		// Create the DataReader
		reader1_ = subscriber_->create_datareader(topic1_, DATAREADER_QOS_DEFAULT, &listener_);
		reader2_ = subscriber_->create_datareader(topic2_, DATAREADER_QOS_DEFAULT, &listener_);

		

		if (reader1_ == nullptr 
			|| 
			reader2_ ==nullptr
			)
		{	
			
			return false;
		}

		return true;
	}

	//!Run the Subscriber
	void run(
		uint32_t samples)
	{
		while (listener_.samples_ < samples)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

};

int main(int argc, char** argv)
{
	std::cout << "Starting subscriber." << std::endl;
	int samples = 5;

	HelloWorldSubscriber* mysub = new HelloWorldSubscriber();
	if (mysub->init())
	{
		mysub->run(static_cast<uint32_t>(samples));
	}

	delete mysub;
	return 0;
}
