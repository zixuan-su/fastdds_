#include "HelloWorldPubSubTypes.h"
#include "PeoplePubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

using namespace eprosima::fastdds::dds;

class HelloWorldPublisher
{
private:

	HelloWorld hello_; 
	People people_;    
	DomainParticipant* participant_;
	

	Publisher* publisher_;

	Topic* topic1_; 
	Topic* topic2_;
	DataWriter* writer1_;
	DataWriter* writer2_;

	TypeSupport type1_;
	TypeSupport type2_;
	class PubListener : public DataWriterListener
	{
	public:

		PubListener()
			: matched_(0)
		{
		}

		~PubListener() override
		{
		}
		//
		void on_publication_matched(DataWriter*,
			const PublicationMatchedStatus& info) override
		{
			
			if (info.current_count_change == 1)
			{
				
				matched_ = info.total_count;
				std::cout << "Publisher matched." << std::endl;
			}
			else if (info.current_count_change == -1)
			{
				
				matched_ = info.total_count;
				std::cout << "Publisher unmatched." << std::endl;
			}
			else
			{
				
				std::cout << info.current_count_change
					<< " is not a valid value for PublicationMatchedStatus current count change." << std::endl;
			}

		}

		std::atomic_int matched_;

	} listener_;

public:

	HelloWorldPublisher()
		: participant_(nullptr)
		, publisher_(nullptr)
		, topic1_(nullptr)
		, topic2_(nullptr)
		, writer1_(nullptr)
		, writer2_(nullptr)
		, type1_(new HelloWorldPubSubType())
		, type2_(new PeoplePubSubType())
	{

	}

	virtual ~HelloWorldPublisher()
	{
		if (writer1_ != nullptr
			||  
			writer2_ != nullptr
			)
		{
			publisher_->delete_datawriter(writer1_);
			publisher_->delete_datawriter(writer2_);
		}
		if (publisher_ != nullptr)
		{
			participant_->delete_publisher(publisher_);
			
		}
		if (topic1_ != nullptr 
			|| topic2_ != nullptr
			)
		{
			participant_->delete_topic(topic1_);
			participant_->delete_topic(topic2_);
		}
		DomainParticipantFactory::get_instance()->delete_participant(participant_);
		
	}

	//!Initialize the publisher
	bool init()
	{
		hello_.index(0);
		hello_.message("this is the topic1		");

		people_.index(0);
	people_.message( "this is the topic2		");

		DomainParticipantQos participantQos;
		participantQos.name("Participant_publisher");
		participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

		if (participant_ == nullptr)
		{
			return false;
		}

		// Register the Type
		type1_.register_type(participant_);
		type2_.register_type(participant_);
		
		

		// Create the publications Topic
		topic1_ = participant_->create_topic("HelloWorldTopic", "HelloWorld", TOPIC_QOS_DEFAULT);
		topic2_ = participant_->create_topic("PeopleTopic", "People", TOPIC_QOS_DEFAULT);
		//

		if (topic1_ == nullptr 
			|| topic2_ == nullptr
			)
		{
			return false;
		}

		// Create the Publisher
		publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

		if (publisher_ == nullptr)
		{
			return false;
		}

		// Create the DataWriter
		writer1_ = publisher_->create_datawriter(topic1_, DATAWRITER_QOS_DEFAULT, &listener_);
		writer2_ = publisher_->create_datawriter(topic2_, DATAWRITER_QOS_DEFAULT, &listener_);
		

		if (writer1_ == nullptr
			|| writer2_ == nullptr
			)
		{
			return false;
		}
		return true;
	}

	//!Send a publication
	bool publish()
	{
		if (listener_.matched_ > 0)
		{
			hello_.index(hello_.index() + 1);
			writer1_->write(&hello_);
			people_.index(people_.index() + 1);
			writer2_->write(&people_);
			return true;
		}
		return false;
	}

	//!Run the Publisher
	void run(
		uint32_t samples)
	{
		uint32_t samples_sent = 0;
		while (samples_sent < samples)
		{
			if (publish())
			{
				samples_sent++;
				std::cout << "Message_HelloWorld: " << hello_.message() << " with Hello  index: " << hello_.index()
					<< " SENT" << std::endl;
				std::cout << "Message_People    : " << people_.message() << " with People index: " << people_.index()
					<< " SENT" << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}

};

int main(int argc, char** argv)
{
	std::cout << "Starting publisher." << std::endl;
	int samples = 5;

	HelloWorldPublisher * mypub = new HelloWorldPublisher();
	if (mypub->init())
	{
		mypub->run(static_cast<uint32_t>(samples));
	}

	delete mypub;
	return 0;
}
